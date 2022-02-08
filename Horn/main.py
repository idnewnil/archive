import horn

engine = horn.Engine([
    'lucky(john)<-',
    '<-study(john)',
    'happy(*X)<-pass(*X,history)^win(*X,lottery)',
    'pass(*Y,*Z)<-study(*Y)',
    'pass(*W,*V)<-lucky(*W)',
    'win(*U,lottery)<-lucky(*U)'
])

process = engine.proof('<-happy(john)')
regular_max_len = max([len(str(each[0])) for each in process])
result_max_len = max([len(str(each[1])) for each in process])

print('证明：<-happy(john)')
for reg, res in process:
    print('结合：', str(reg).center(regular_max_len), '，生成：', str(res).center(regular_max_len))
