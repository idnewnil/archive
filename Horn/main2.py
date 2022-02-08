import horn
import datetime

start = datetime.datetime.now()
engine = horn.Engine([
    '父亲(贾代化,贾政)<-',
    '父亲(贾代化,贾敏)<-',
    '父亲(贾政,贾宝玉)<-',
    '母亲(贾敏,林黛玉)<-',
    '男性(贾宝玉)<-',
    '女性(林黛玉)<-',
    '表兄妹(*X,*Y)<-父子(*U,*X)^母女(*V,*Y)^兄妹(*U,*V)^表兄妹(*X,*Y)',
    '父子(*X,*Y)<-男性(*X)^男性(*Y)^亲子(*X,*Y)',
    '母女(*X,*Y)<-女性(*X)^女性(*Y)^亲子(*X,*Y)',
    '父女(*X,*Y)<-男性(*X)^女性(*Y)^亲子(*X,*Y)',
    '兄妹(*X,*Y)<-父子(*U,*X)^父女(*U,*Y)',
    '男性(*X)<-父亲(*X,*Y)',
    '亲子(*X,*Y)<-父亲(*X,*Y)',
    '女性(*X)<-母亲(*X,*Y)',
    '亲子(*X,*Y)<-母亲(*X,*Y)'
])
process = engine.proof('<-表兄妹(贾宝玉,林黛玉)')

print('证明： <-表兄妹(贾宝玉,林黛玉)')
for reg, res in process:
    print('结合：', str(reg))
    print('生成：', str(res))
