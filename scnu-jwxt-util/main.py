import bs4
import base64
import pandas as pd
import re
import requests
import rsa
import time


def timestamp():
    return int(time.time() * 1000)


class ScnuJwxtLoginForm:
    class InvalidAccountOrPasswordError(Exception):
        pass

    def __init__(self):
        self.username = ""
        self.password = ""
        self.time = timestamp()
        self.session = requests.session()
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) '
                          'Chrome/84.0.4147.89 Safari/537.36 '
        })

    @staticmethod
    def hextob64(s):
        return str(base64.b64encode(s), 'utf-8')

    @staticmethod
    def b64tohex(s):
        return base64.b64decode(s).hex()

    # 用户登录
    def login(self):
        # 1、访问页面，获取csrftoken
        resp = self.session.get(
            'https://jwxt.scnu.edu.cn/xtgl/login_slogin.html')
        soup = bs4.BeautifulSoup(resp.text, "lxml")
        csrftoken = soup.find('input', id='csrftoken')['value']
        # 2、获取密钥信息
        rsa_key_info = self.session.get(
            f'https://jwxt.scnu.edu.cn/xtgl/login_getPublicKey.html?time={self.time}').json()
        modulus = int(ScnuJwxtLoginForm.b64tohex(rsa_key_info['modulus']), 16)
        exponent = int(ScnuJwxtLoginForm.b64tohex(
            rsa_key_info['exponent']), 16)
        pub_key = rsa.PublicKey(modulus, exponent)
        # 3、公钥加密密码
        enpsw = ScnuJwxtLoginForm.hextob64(
            rsa.encrypt(self.password.encode('utf-8'), pub_key))
        # 4、登录
        resp = self.session.post(f'https://jwxt.scnu.edu.cn/xtgl/login_slogin.html?time={self.time}', headers={
            'Content-Type': 'application/x-www-form-urlencoded'
        }, data=[('csrftoken', csrftoken), ('yhm', self.username), ('mm', enpsw), ('mm', enpsw)])
        if re.match('用户名或密码不正确', resp.text):
            raise ScnuJwxtLoginForm.InvalidAccountOrPasswordError


class ScnuJwxt:
    def __init__(self, sjlf: ScnuJwxtLoginForm):
        self.username = sjlf.username
        self.session = sjlf.session

    def query_all_score(self):
        resp = self.session.post('https://jwxt.scnu.edu.cn/cjcx/cjcx_cxDgXscj.html?doType=query&gnmkdm=N305005', data={
            'xnm': '',
            'xqm': '',
            '_search': False,
            'nd': int(time.time() * 1000),
            'queryModel.showCount': 5000,
            'queryModel.currentPage': 1,
            'queryModel.sortName': '',
            'queryModel.sortOrder': 'asc',
            'time': 1
        })
        json_ = resp.json()
        df = pd.DataFrame()
        # 转换成dataframe
        for i, item in enumerate(json_['items']):
            for k, v in item.items():
                # 把可计算类型的值转换成对应类型
                if k in ['cj', 'xnm', 'xqm', 'xqmmc']:
                    v = int(v)
                elif k in ['jd', 'xf', 'xfjd']:
                    v = float(v)
                elif isinstance(v, dict):
                    continue
                df.loc[i, k] = v
        # 计算学期数
        df['xqs'] = (df['xnm'] - df['xnm'].min()) * 2 + df['xqmmc']
        return df


def gpa(df: pd.DataFrame, terms):
    test = False
    for term in terms:
        test = test | (df['xqs'] == term)
    filter_result = df[test]
    return filter_result['xfjd'].sum() / filter_result['xf'].sum()


def gpas(df: pd.DataFrame):
    xqs = df['xqs'].unique()
    every_term = {}
    for xq in xqs:
        every_term[int(xq)] = gpa(df, (xq,))
    every_year = {}
    for i in range(len(xqs) // 2):
        every_year[i + 1] = gpa(df, (2*i+1, 2*i+2))
    total = gpa(df, xqs)
    return every_term, every_year, total


def main():
    import sys
    sjlf = ScnuJwxtLoginForm()
    sjlf.username = input('用户名：')
    sjlf.password = input('密码：')
    sjlf.login()
    sj = ScnuJwxt(sjlf)
    df = sj.query_all_score()
    every_term, every_year, total = gpas(df)
    print('每学期绩点：')
    print(every_term)
    print('每学年绩点')
    print(every_year)
    print('总绩点：', total)
    range_size = 10
    cj = df['cj']
    for a in range(0, 100, range_size):
        b = a + range_size - 1
        print(f'{a}-{b}分的科目的数量：', ((a <= cj) & (cj <= b)).sum())
    print('100分科目的数量：', (cj == 100).sum())


if __name__ == '__main__':
    main()
