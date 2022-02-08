# 文档介绍

## 一些字段的意义

`appid`：用来区分一个应用是否是我们自身开发的，如果没有这个字段，那么发送短信的功能只要调用api就能随便使用，存在风险。

## 如何查看

在文档中，会用`$param`表示param是一个变量，并在下方给出其意义。`$param...`指的是param为可变参数，即同时指代`$param1`，`$param2`，...，`$paramN`多个参数。

引用文档时，会用**$key**=value的形式给出，value不一定是常量，其中可能也会定义新变量。

文档会给出测试时的一些数据，每个HTTP报文的交互作为一块显示。

```
==================== REQUEST =====================
方法 URL

发送数据
==================== RESPONSE ====================
Http状态码 状态字符串

返回数据
====================== END =======================
```

## 数据类型

### Point

一个点可以用来标记用户、商品、任务目标的位置，其结构如下

```
{
	"longitude": $longitude,
	"latitude": $latitude,
	"name": $name
}
```

`$longitude`：经度，范围为-180到180

`$latitude`：纬度，范围为0到90

`$name`：该地点的名字，可以为空。

**注意：**由于特殊原因，**$longitude**=0且**$latitude**=90的点将会被看作为`null`。

# 发送验证码

在访问一些需要验证码验证的链接时，需要首先发送验证码。

```
POST http://api.yilao.tk:5000/v1.0/sms

{
	"appid": "df3b72a07a0a4fa1854a48b543690eab",
	"mobile": $mobile,
	"method": $method,
	"path": $path
}
```

`$mobile`**：用户手机号码**

`$method $path`**：需要访问的链接**

注意，path并不包含参数部分。

##### 返回

`201`：成功

# 用户

|       属性       |  类型  |           限制           |         解释         |
| :--------------: | :----: | :----------------------: | :------------------: |
|      mobile      |  整数  |       手机号码格式       |        手机号        |
|      passwd      | 字符串 |                          |         密码         |
|     nickname     | 字符串 |                          |         昵称         |
|       sex        | 字符串 |       male或female       |         性别         |
|     portrait     |  整数  | 必须是自己所拥有的资源id |         头像         |
| default_location | Point  |                          |     用户默认地址     |
|    create_at     | 字符串 |                          | 注册日期（无需上传） |
|     id_name      | 字符串 |                          |      认证的名字      |
|    id_school     | 字符串 |                          |      认证的学校      |
|     id_photo     | 字符串 |     mobile=0的资源ID     |      认证的图片      |

## 注册

##### 1. 发送验证码

**$method**=`PUT`

**$path**=`/v1.0/users/$mobile`

##### 2. 验证并注册

```
PUT http://api.yilao.tk:5000/v1.0/users/$mobile?appid=df3b72a07a0a4fa1854a48b543690eab&code=$code

{
	$key1: $value1, 
	$key2: $value2,
	...
}
```

`$mobile`**：用户手机号码**

`$code`**：验证码**

`$key...`**：用户字段（passwd字段为必须的）**

`$value...`**：字段值**

##### 返回

`201`：成功

`401`：用户验证失败

## 登陆

用户登陆采取token的方式，通过密码或者验证码来获得token。在设计中token拥有权限域，在使用api时不同的功能可能需要不同的权限，不过目前token只有全部权限这个选择。

### 通过验证码

##### 1. 发送验证码

**$method**=`POST`

**$base_url**=`http://api.yilao.tk:5000/v1.0/users/$mobile/tokens`

##### 2. 验证获取token

```
POST http://api.yilao.tk:5000/v1.0/users/$mobile/tokens?appid=df3b72a07a0a4fa1854a48b543690eab&code=$code
```

`$mobile`**：用户手机号码**

`$code`**：验证码**

##### 返回

`201`：成功，并且token会以json格式返回，

```
{
	"token": $token
}
```

`$token`**：令牌**

`401`：用户验证失败

### 通过密码

```
POST http://api.yilao.tk:5000/v1.0/users/$mobile/tokens?appid=df3b72a07a0a4fa1854a48b543690eab&passwd=$passwd
```

`$passwd`**：用户密码**

##### 返回

同上

## 更新

更新用户的信息，有一些字段如手机号、注册时间等会被拒绝更新。

### 更新非重要字段

```
PATCH http://api.yilao.tk:5000/v1.0/users/$mobile?appid=df3b72a07a0a4fa1854a48b543690eab&token=$token

{
	$key1: $value1, 
	$key2: $value2,
	...
}
```

`$key...`**：用户字段**

`$value...`**：字段值（不能包含passwd等重要字段）**

`$token`**：令牌**

##### 返回

`204`：成功

`401`：用户验证失败

### 更新重要字段

更新一些比较重要的字段时，不能使用token来验证，需要进行密码验证或者短信验证。更新过程基本和非重要字段一致，相应的，短信验证需要先发送短信，并且`token=$token`也应该改为`code=$code`或者`passwd=$passwd`。

`$code`：验证码

`$passwd`：密码

##### 返回

同上

## 获取

```
GET http://api.yilao.tk:5000/v1.0/users/$mobile
```

##### 返回

`200`：成功，结果包含用户的所有信息。

`404`：用户不存在

# 订单

|      属性       |  类型  |      限制      |                             解释                             |
| :-------------: | :----: | :------------: | :----------------------------------------------------------: |
|       id        |  整数  |                |                      订单id（无需上传）                      |
|    from_user    |  整数  |  手机号码格式  |                哪个用户创建的订单（无需上传）                |
|      phone      |  整数  |  手机号码格式  |                            联系人                            |
|   destination   | Point  |                |                   完成所有任务后去哪里交付                   |
| emergency_level | 字符串 | normal或urgent |                           紧急程度                           |
|    create_at    | 字符串 |                |                   创建订单时间（无需上传）                   |
|   receive_at    | 字符串 |                |                   接受订单时间（无需上传）                   |
|    executor     |  整数  |  手机号码格式  |                            执行者                            |
|    close_at     | 字符串 |                |                   关闭订单时间（无需上传）                   |
|   close_state   | 字符串 | finish或cancel |                          关闭的状态                          |
|      type       | 字符串 |                |                           任务类型                           |
|    category     | 字符串 |                |                           二级类别                           |
|     detail      | 字符串 |                |                           任务描述                           |
| protected_info  | 字符串 |                |                 接单后才能告诉别人的私密信息                 |
|     photos      | 字符串 |                |                         多张照片的id                         |
|      count      |  整数  |                |                           执行次数                           |
|     reward      | 浮点数 |                |                             奖赏                             |
|      in_at      | 字符串 |                | 用户收回资源的时间（同理，资源使用完成后用户可能会有一个收回的时候） |
|     out_at      | 字符串 |                | 用户借出资源的时间（做某项任务时可能需要用户给的某些资源，因此有一个借出时间） |
|    id_photo     | 字符串 |                |                   下单者的头像（无需上传）                   |
|      name       | 字符串 |                |                            订单名                            |

## 新建

```
POST http://127.0.0.1:5000/v1.0/users/$mobile/orders?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab

{
	$key1: $value1, 
	$key2: $value2,
	...
}
```

`$mobile`：手机号码

`$token`：登陆时获取到的令牌

`$key...`**：字段**

`$value...`**：值**

##### 返回

`200`：成功

## 更新

### 接单/取消接单

```
PATCH http://127.0.0.1:15000/v1.0/users/$mobile/orders/$order_id?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab&receive=$receive
```

`$mobile`：接单者的手机号码

`$order_id`：需要接单的订单id

`$token`：登陆时获取到的令牌

`$receive`：取值`true`或者`false`，是接单还是取消接单，取消接单只有接单不超过三分钟才能进行。

### 完成/取消订单

```
PATCH http://127.0.0.1:15000/v1.0/users/$mobile/orders/$order_id?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab&close=$close
```

`$mobile`：手机号码

`$order_id`：订单id

`$token`：登陆时获取到的令牌

`$close`：取值`finish`或者`cancel`，是完成还是取消订单。如果是取消订单，且已接单那么会进入canceling状态，等待对方同意。

## 接受/拒绝取消订单

```
PATCH http://127.0.0.1:15000/v1.0/users/$mobile/orders/$order_id?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab&close=close
```

`$mobile`：接单者的手机号码

`$order_id`：需要接单的订单id

`$token`：登陆时获取到的令牌

`$receive`：取值`true`或者`false`，是接单还是取消接单，取消接单只有接单不超过三分钟才能进行。

`$close`：取值`close`或`reopen`，是接受还是拒绝。

## 获取

### 可接取的订单

```
GET http://127.0.0.1:5000/v1.0/public_orders?begin=$begin&end=$end&type=type&mobile=$mobile
```

`$begin`：整数，以现在为基准的时间偏移，单位为秒

`$end`：整数，同上

`$type`：订单类别

`$mobile`：获取订单的用户，服务端将不会返回该用户的订单

时间筛选举例：如果需要选择3分钟前到2分钟前的订单，则$begin=-3\*60=-180（注意要有负号），\$end=-2\*60=-120

##### 返回

`200`：成功，并且返回以下的json

```
[
	{
		订单1内容
	},
	{
		订单2内容
	},
	...
]
```

### 和自己有关的订单

```
GET http://127.0.0.1:5000/v1.0/users/$mobile/orders?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab
```

`$mobile`：手机号

`$token`：令牌

##### 返回

同上

# 聊天记录

|   属性    |  类型  |           解释           |
| :-------: | :----: | :----------------------: |
|    id     |  整数  |      id（无需上传）      |
|  content  | 字符串 |           内容           |
| from_user |  整数  | 来自哪位用户（无需上传） |
|  to_user  |  整数  |      发送给哪位用户      |
|  send_at  | 字符串 |   发送时间（无需上传）   |

## 发送

```
POST http://127.0.0.1:5000/v1.0/users/$mobile/dialogs?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab

{
    "content": $content,
    "to_user": $to_user
}
```

`$mobile`：手机号

`$token`：令牌

`$content`：要发送的内容

`$to_user`：要发送的用户，为0则获取与所有用户的信息

##### 返回

`201`：成功

## 获取和自己通讯过的所有用户

```
GET http://127.0.0.1:5000/v1.0/users/$mobile/dialogs_users?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab
```

`$mobile`：手机号

`$token`：令牌

##### 返回

`201`：成功

```
[
	{
		“from_user" : 0,
		"to_user": 0,
		"last_send_at": "",
		"last_content": ""
	}
]
```

## 获取

```
GET http://127.0.0.1:5000/v1.0/users/$mobile/dialogs_with/$another_user?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab&min_id=$min_id
```

`$mobile`：手机号

`$another_user`：另一个用户

`$token`：令牌

`$min_id`：筛选id，如果id=0则启用receive=true

##### 返回

`200`：成功，并且返回以下的json

```
[
	{
		聊天记录1
	},
	{
		聊天记录2
	},
	...
]
```

# 资源

|   属性    |  类型  |           解释           |
| :-------: | :----: | :----------------------: |
|   uuid    | 字符串 |     uuid（无需上传）     |
|   name    | 字符串 |           名字           |
| from_user |  整数  | 来自哪位用户（无需上传） |
| create_at | 字符串 |   创建时间（无需上传）   |

## 新建

url为`POST http://127.0.0.1:5000/v1.0/users/$mobile/resources?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab`，具体上传文件请参考使用表单上传文件，格式`multipart/form-data`

`$mobile`：手机号或0，mobile=0时无需进行认证

`$token`：令牌

##### 返回

`201`：成功，并且会返回其uuid

```
{
	"uuid": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
}
```

## 下载

```
GET http://127.0.0.1:5000/v1.0/users/$mobile/resources/$uuid
```

`$mobile`：手机号

##### 返回

`200`：成功，并且文件会在body中返回

# 商品

|   属性    |    类型    |     解释     |
| :-------: | :--------: | :----------: |
|    id     |    整数    |      id      |
|   name    |   字符串   |   商品名字   |
|  detail   |   字符串   |   商品描述   |
| from_user |    整数    | 来自哪位用户 |
| location  |   Point    |     定位     |
| on_offer  |    bool    | 是否还在售卖 |
|   price   |   浮点数   |     价格     |
|   photo   | 资源的uuid |   商品图片   |

## 新建

```
POST http://127.0.0.1:15000/v1.0/users/$mobile/commodities?token=$token&appid=df3b72a07a0a4fa1854a48b543690eab

{
    $key1: $value1, 
	$key2: $value2,
	...
}
```

`$mobile`：手机号码

`$token`：登陆时获取到的令牌

`$key...`**：字段**

`$value...`**：值**

## 获取

```
GET http://127.0.0.1:15000/v1.0/commodities/$commodity_id
```

`$commodity_id`：商品的id