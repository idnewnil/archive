import json
import requests
import bs4
import os

cookies = {}
with open('cookies.json') as f:
    for item in json.load(f):
        name = item['name']
        value = item['value']
        cookies[name] = value

s = requests.session()
s.cookies.update(cookies)

resp = s.get('https://moodle.scnu.edu.cn/course/view.php?id=8028')

topics = bs4.BeautifulSoup(resp.content.decode(), 'lxml').find(class_='topics')
# 第一章从下标为1开始
contents = topics.find_all(class_='content')[1:]
for content_i, content in enumerate(contents):
    content_i += 1
    # 章节名
    section_name = content.find(class_='sectionname').find('span').contents[0]
    # 章节内容
    section = content.find(class_='section')
    # 创建对应文件夹
    filepath = f'./{content_i} {section_name}'
    if not os.path.exists(filepath):
        os.makedirs(filepath)
    if section:
        # 模块
        for module_i, module in enumerate(section):
            module_i += 1
            print(f'处理第{content_i}章，第{module_i}模块')
            aalink = module.find(class_='aalink')
            if aalink:
                aalink = aalink['href']
            instance_name = module.find(class_='instancename')
            if instance_name:
                instance_name = instance_name.contents[0]

            # 该模块是测试
            if 'quiz' in module['class']:
                aalink_resp = bs4.BeautifulSoup(s.get(aalink).content.decode(), 'lxml')
                review_link = aalink_resp.select('.lastrow .lastcol')[0].a['href']
                review = bs4.BeautifulSoup(s.get(review_link).content.decode(), 'lxml')
                filename = f'{module_i}. {instance_name}.html'
                with open(f'{filepath}/{filename}', 'w', encoding='utf8') as f:
                    f.write(str(review))
            # 判断为可下载的资源
            elif 'resource' in module['class'] or 'url' in module['class']:
                aalink += '&redirect=1'
                resource_resp = s.get(aalink, stream=True)
                filename = resource_resp.headers['Content-disposition'].encode('ISO-8859-1').decode('utf8')
                filename = filename.split('filename=')[1][1:-1]
                filename = f'{module_i}. {filename}'
                fullname = f'{filepath}/{filename}'
                if (not os.path.exists(fullname) or
                        str(os.path.getsize(fullname)) != resource_resp.headers['content-length']):
                    with open(fullname, 'wb') as f:
                        for chunk in resource_resp.iter_content(chunk_size=1024 * 1024):
                            if chunk:
                                f.write(chunk)
                else:
                    resource_resp.close()
            elif 'page' in module['class']:
                with open(f'{filepath}/{module_i} ----无法下载----', 'w', encoding='utf8') as f:
                    pass
            elif 'label' in module['class']:
                with open(f'{filepath}/{module_i} ----分割线----', 'w', encoding='utf8') as f:
                    pass
            elif 'assign' in module['class']:
                with open(f'{filepath}/{module_i} ----作业----', 'w', encoding='utf8') as f:
                    pass
            else:
                raise Exception('未知类型：', module['class'])
