from datetime import datetime, timedelta
from flask import request, Response
from flask_restful import Resource
from schema import order_schema
from wrap import _use
from ..util import dump_locations, curd_params, exc, mycursor, null_point, get_now
from .validation import token_validate
from typing import List


class PublicOrderListResource(Resource):
    def get(self):
        """获取所有可以接取的任务"""
        _use(self)
        with mycursor(dictionary=True) as c:
            c.execute(
                "select *, u.id_photo, u.id_name from `order` left join user u on u.mobile = `order`.from_user "
                "where executor is null and close_state is null and u.id_school=`order`.destination_name")
            order_list = c.fetchall()
            for order in order_list:
                dump_locations(order, order_schema)
            return order_schema.dump(OrderListResource.filter_(order_list), many=True)


class OrderListResource(Resource):
    @staticmethod
    def filter_(orders: List[dict]):
        now = get_now()
        try:
            begin = now + timedelta(seconds=int(request.args.get('begin')))
        except TypeError:
            begin = None
        try:
            end = now + timedelta(seconds=int(request.args.get('end')))
        except TypeError:
            end = None
        type_ = request.args.get('type')
        category = request.args.get('category')
        mobile = request.args.get('mobile')
        res = []
        for order in orders:
            create_at = order['create_at']
            if begin is not None and create_at < begin:
                continue
            if end is not None and create_at > end:
                continue
            if type_ is not None and order['type'] != type_:
                continue
            if category is not None and order['category'] != category:
                continue
            if mobile is not None and order['mobile'] == mobile:
                continue
            res.append(order)
        return res

    @token_validate
    def get(self, mobile):
        """获取和自己相关的订单，自己是发布者或者自己是执行者"""
        with mycursor(dictionary=True) as c:
            c.execute('select *, u.id_photo id_photo, u.id_name id_name, '
                      'u1.id_photo id_photo1, u1.id_name id_name1 from `order` '
                      'left join user u on `order`.from_user = u.mobile '
                      'left join user u1 on `order`.executor=u1.mobile '
                      'where from_user=%s or executor=%s', (mobile, mobile))
            order_list = c.fetchall()
            for order in order_list:
                dump_locations(order, order_schema)
            return order_schema.dump(self.filter_(order_list), many=True)

    @token_validate
    def post(self, mobile):
        """新建一个订单"""
        order = order_schema.loads(request.data.decode())
        order['from_user'] = mobile
        if 'destination' not in order:
            order['destination'] = null_point

        with mycursor() as c:
            to_insert = curd_params(order, order_schema)
            c.execute(f'insert into `order` set {to_insert[0]}', to_insert[1])
            order_id = c.lastrowid

        return order_schema.dump({'id': order_id}), 201


class OrderResource(Resource):
    @token_validate
    def patch(self, mobile, order_id):
        """更新订单状态，注意patch只做订单被接受和订单被关闭的处理
        如果要修改订单信息，请使用delete+post"""
        with mycursor(dictionary=True) as c:
            c.execute('select * from `order` where id=%s limit 1', (order_id,))
            order = c.fetchone()
            from_user = order['from_user']
            receive = request.args.get('receive')
            receive = receive.lower() if isinstance(receive, str) else receive
            close = request.args.get('close')
            if mobile != from_user:
                # 不是发布者，只能更新是否接受任务状态或者接受对方取消订单
                if receive == 'true':
                    # 如果要接单
                    if order['executor'] is None:
                        c.execute('update `order` set receive_at=current_timestamp, executor=%s where id=%s',
                                  (mobile, order_id))
                    else:
                        return exc('该订单已被接受'), 400
                elif receive == 'false':
                    # 如果要取消接单
                    if order['executor'] == mobile:
                        # 如果接受者是自己，那么三分钟内可以取消
                        if get_now() - order['receive_at'] < timedelta(minutes=3):
                            c.execute('update `order` set receive_at=null, executor=null where id=%s', (order_id,))
                        else:
                            return exc('超过三分钟，无法取消接单'), 400
                    else:
                        return exc('订单已被其他人接受'), 400
                elif order['close_state'] == 'canceling':
                    if close == 'close':
                        c.execute("update `order` set close_at=current_timestamp, close_state='cancel' where id=%s",
                                  (order_id,))
                    elif close == 'reopen':
                        c.execute("update `order` set close_at=null, close_state=null where id=%s", (order_id,))
                    else:
                        return exc('错误的值close'), 400
                else:
                    return exc('错误的值close'), 400
            else:
                # 否则是更新关闭订单的信息
                if order['close_state'] is not None:
                    return exc('订单已关闭'), 400
                close = request.args.get('close')
                if receive:
                    return exc('无效参数receive'), 400
                if close is None:
                    return exc('缺少参数close'), 400
                if close == 'cancel':
                    # 希望取消订单
                    if order['executor'] is not None:
                        # 已有人接单，改为取消中
                        close = 'canceling'
                c.execute('update `order` set close_at=current_timestamp, close_state=%s where id=%s',
                          (close, order_id))
            return Response(status=204)
