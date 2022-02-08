# Package       Version
# python        3.8
# matplotlib    3.3.3
# numpy         1.19.2
# scipy         1.5.4

from functools import partial
import matplotlib.pyplot as plt
import numpy as np
import random
from scipy.optimize import linear_sum_assignment
from time import time
import sys

np.set_printoptions(threshold=np.inf, linewidth=np.inf, suppress=True)
plt.rcParams['font.family'] = 'STSong'
plt.rcParams['font.size'] = 14
random.seed(0)
np.random.seed(0)


class Error(Exception):
    pass


class InvalidParamError(Error):
    pass


class RouletteError(Error):
    pass


class NoAvailableNodeError(Error):
    pass


class InvalidProblemError(Error):
    pass


def neighbor_pair(data: list, circle=True):
    for i in range(1 - circle, len(data)):
        yield data[i - 1], data[i]


# 轮盘赌
def roulette(p_list):
    sump = sum(p_list)
    p_list = [it / sump for it in p_list]
    rnd = random.random()
    cnt = 0
    for i, p in enumerate(p_list):
        cnt += p
        if cnt >= rnd:
            return i
    raise RouletteError


inf = np.inf
nan = np.nan
isnan = np.isnan


def hungarian_method(cost):
    row_ind, col_ind = linear_sum_assignment(cost)
    return cost[row_ind, col_ind].sum()


def greedy_search(cost: np.ndarray):
    """
    贪婪算法搜索AP问题，不通用tsp
    :param cost: 效益矩阵
    :return: 贪婪算法求得的结果
    """
    if not isinstance(cost, np.ndarray):
        cost = np.array(cost)
    assert cost.ndim == 2
    res = 0
    while cost.shape[0] != 0 and cost.shape[1] != 0:
        min_pos_y, min_pos_x = divmod(cost.argmin(), cost.shape[1])
        res += cost[min_pos_y, min_pos_x]
        cost = np.delete(cost, min_pos_y, axis=0)
        cost = np.delete(cost, min_pos_x, axis=1)
    return res


def greedy_search2(d: np.ndarray):
    """
    贪婪算法
    :param d:
    :return:
    """
    assert d.ndim == 2 and d.shape[0] == d.shape[1]
    m = d.shape[0]

    # 根据贪婪的规则，最初的两个点的路径应是最短的
    path = [*divmod(d.argmin(), d.shape[1])]
    path_dist = np.min(d)

    j_set = set(range(m))
    for i in range(m - 2):
        i = path[-1]
        min_dist = inf
        min_dist_j = -1
        for j in j_set:
            if isnan(d[i, j]) or d[i, j] == inf:
                continue
            if d[i, j] < min_dist:
                min_dist = d[i, j]
                min_dist_j = j
        path.append(min_dist_j)
        path_dist += min_dist
        j_set.remove(min_dist_j)
    return path, path_dist


# 指派问题转换为TSP问题
def ap2tsp(cost):
    cost = np.array(cost)
    assert cost.ndim == 2
    m = cost.shape[0]
    n = cost.shape[1]

    res = np.empty((m + n, m + n))
    for i in range(m):
        for j in range(n):
            res[i, m + j] = cost[i, j]  # 智能体到任务的值为相应效益矩阵的值
            res[m + j, i] = 0  # 任务到智能体的距离为0

    for i in range(m):
        for j in range(m):
            res[i, j] = nan  # 智能体到智能体之间不存在距离

    for i in range(n):
        for j in range(n):
            res[m + i, m + j] = nan  # 任务到任务间不存在距离

    return res


# 指派问题Map
class APMap:
    def __init__(self, d: np.ndarray, Cnn, rho, theta, ksi, is_ap):
        """
        :param d: 效益矩阵，inf代表距离无限长，所有值都应该是非负数
        :param Cnn: 贪婪算法构建的路径长度
        :param rho: 信息素全局蒸发率
        :param theta: 0偏移，用于确定zero的实际值
        :param ksi: 信息素局部挥发速率
        """
        self.cost_shape = d.shape
        self.full_path_len = min(self.cost_shape) * 2
        self.n = sum(d.shape)
        self.d = ap2tsp(d) if is_ap else d
        self.node_count = self.d.shape[0]
        if self.d.ndim != 2 or self.d.shape[0] != self.d.shape[1]:
            raise InvalidParamError
        self.d_max = np.max(self.d[np.where(~isnan(self.d))])
        self.zero = 0
        self._0 = min(1, np.min(np.abs(self.d))) / 1e5
        if theta > 0:
            self.zero = min(1, np.min(np.abs(self.d))) / theta
        self.d[self.d == 0] = self.zero

        self.rho = rho
        self.Cnn = Cnn
        if self.Cnn == 0:
            self.Cnn = self.zero if self.zero != 0 else self._0

        # 计算tau0和tau
        self.tau0 = 1 / (self.n * self.Cnn)
        self.tau: np.ndarray = self.tau0 * np.ones(self.d.shape)

        # 设置可接受的初始节点
        self.accepted_initial_nodes = range(self.cost_shape[0])

        self.ksi = ksi

        self.best_path = None
        self.best_path_dist = inf

    # 根据地图计算给定路径距离
    def dist(self, path):
        assert len(path) == min(self.cost_shape) * 2
        res = 0
        for i, j in neighbor_pair(path):
            value = self.d[i, j]
            if value == self.zero:
                value = 0
            res += value
        return res

    def pheromone_global_update(self):
        tau = self.tau
        rho = self.rho
        Cb = self.best_path_dist
        if Cb == 0:
            Cb = self.zero if self.zero != 0 else self._0
        delta_taub = 1 / Cb
        for i, j in neighbor_pair(self.best_path):
            tau[i, j] = (1 - rho) * tau[i, j] + rho * delta_taub

    def pheromone_partial_update(self, i, j):
        tau = self.tau
        ksi = self.ksi
        tau0 = self.tau0
        tau[i, j] = (1 - ksi) * tau[i, j] + ksi * tau0


class Ant:
    def __init__(self, map_: APMap, alpha, beta, q0):
        """
        :param map_: 蚂蚁所在地图
        :param alpha: 启发式信息比重
        :param beta: 信息素浓度比重，当alpha=0时，蚂蚁相当于采取贪婪算法，当beta=0时，蚂蚁只根据信息素浓度确定路径
        :param q0: 开发概率
        """
        self.map = map_
        self.alpha = alpha
        self.beta = beta
        self.q0 = q0
        self.path = []
        self.best_path = None
        self.best_path_dist = inf

    def clear_and_choose_initial_node(self):
        self.path = []
        ain = self.map.accepted_initial_nodes
        self.path.append(random.choice(ain))

    def choose_next_node(self):
        tau = self.map.tau
        d = self.map.d
        i = self.path[-1]

        # 可选的下一个节点
        jk = set(np.where(~isnan(d[i]))[0]) - set(self.path)
        jk = list(jk)

        # 计算下一个节点的概率
        taui = tau[i, jk]
        di = d[i, jk]
        with np.errstate(divide='ignore'):
            etai_beta = (1 / di) ** self.beta

        q = random.random()
        if q < self.q0:
            # 开发
            j = jk[int(np.argmax(np.maximum(taui, etai_beta)))]
        else:
            # 探索
            if self.map.zero == 0 and np.any(etai_beta == inf):
                etai_beta[etai_beta != inf] = 0
                etai_beta[etai_beta == inf] = 1
            numerator = [t ** self.alpha * eb for t, eb in zip(taui, etai_beta)]
            denominator = sum(numerator)
            if denominator == 0:
                # 如果分母为0，说明分子全为0，对应路径无穷大，因此继续走没有意义了
                raise NoAvailableNodeError
            p_list = [it / denominator for it in numerator]

            # 轮盘赌选择下一个节点
            j = jk[roulette(p_list)]

        self.path.append(j)


class Record:
    def __init__(self, begin_time, end_time, tau, curr_best_path, curr_best_path_dist, best_path, best_path_dist):
        self.dt = (end_time - begin_time) * 1000
        self.idt = int(self.dt)
        self.tau = tau
        self.cbp = curr_best_path
        self.cbpd = curr_best_path_dist
        self.bp = best_path
        self.bpd = best_path_dist


class ACS:
    def __init__(self, d, m, Cnn, rho, alpha, beta, theta, ksi, q0, is_ap=True):
        if not isinstance(d, np.ndarray):
            d = np.array(d)
        self.map = APMap(d, Cnn, rho, theta, ksi, is_ap)
        self.m = m
        self.ants = [Ant(self.map, alpha, beta, q0) for _ in range(m)]

    def iter(self):
        begin_time = time()
        # 并行工作
        for ant in self.ants:
            ant.clear_and_choose_initial_node()
        skip_ant = set()
        for i in range(self.map.full_path_len - 1):
            for ant in set(self.ants) - skip_ant:
                try:
                    ant.choose_next_node()
                    # 蚂蚁选择路后立即进行局部更新
                    self.map.pheromone_partial_update(ant.path[-2], ant.path[-1])
                except NoAvailableNodeError:
                    # 这只蚂蚁走了一条死胡同，放弃继续寻路
                    skip_ant.add(ant)
        curr_best_path = None
        curr_best_path_dist = inf
        for ant in set(self.ants) - skip_ant:
            # 找到当前最优路径
            dist = self.map.dist(ant.path)
            if dist < curr_best_path_dist:
                curr_best_path = ant.path
                curr_best_path_dist = dist
        if curr_best_path_dist < self.map.best_path_dist:
            # 更新全局最优路径
            self.map.best_path = curr_best_path
            self.map.best_path_dist = curr_best_path_dist
        # 信息素全局更新
        self.map.pheromone_global_update()
        end_time = time()
        return Record(begin_time, end_time, self.map.tau.copy(), curr_best_path, curr_best_path_dist,
                      self.map.best_path, self.map.best_path_dist)


def acs_solution(d, m, Cnn, rho, alpha, beta, theta, ksi, q0, n_iter_no_change, max_iter):
    acs = ACS(d, m, Cnn, rho, alpha, beta, theta, ksi, q0)
    min_dist = acs.map.best_path_dist
    curr_iter_no_change = 0
    n = 0
    while curr_iter_no_change < n_iter_no_change and n < max_iter:  # 如果最优路径经过n次迭代不变或超过了max_iter次迭代才会停止循环
        yield n, acs.iter()
        if acs.map.best_path_dist < min_dist:
            min_dist = acs.map.best_path_dist
            curr_iter_no_change = 0
        else:
            curr_iter_no_change += 1
    if curr_iter_no_change != n_iter_no_change:
        print('已达到最大迭代次数，但没有收敛', file=sys.stderr)


# 随机生成效益矩阵，可以设置0和inf出现的概率
def random_cost(m: int, n: int, p_zero=.05, p_inf=.0):
    if m < 0:
        m = random.randint(1, -m)
    if n < 0:
        n = random.randint(1, -n)
    if p_zero < 0:
        p_zero = random.random() * -p_zero
    if p_inf < 0:
        p_inf = random.random() * -p_inf
    assert m != 0 and n != 0 and p_zero >= 0 and p_inf >= 0 and p_zero + p_inf < 1

    res = np.random.random((m, n))
    res[res < p_zero] = 0
    res[res > 1 - p_inf] = inf
    to_scale = (res != 0) & (res != inf)
    res[to_scale] = (res[to_scale] - p_zero) / (1 - p_zero - p_inf)
    return res


# 测试
def test(d, m, rho, alpha, beta, thetas, ksi, q0, n_iter_no_change=10, max_iter=200, verbose=1, draw=False, imgpath=''):
    """
    :param d:
    :param m:
    :param rho:
    :param alpha:
    :param beta:
    :param thetas:
    :param ksi:
    :param q0:
    :param n_iter_no_change:
    :param max_iter:
    :param verbose: 控制输出的详细程度
    :param draw: 是否绘画图片
    :param imgpath: 图片输出路径
    :return:
    """
    begin_time = time()
    Cnn = greedy_search(d)
    end_time = time()
    Cnn_delta_time = int((end_time - begin_time) * 1000)
    begin_time = time()
    km = hungarian_method(d)
    end_time = time()
    km_delta_time = int((end_time - begin_time) * 1000)
    worst_path_dist = -hungarian_method(-d)
    tau0 = 1 / (sum(d.shape) * Cnn)
    if worst_path_dist == 0:
        raise

    def test0(theta):
        finder = acs_solution(d, m, Cnn, rho, alpha, beta, theta, ksi, q0, n_iter_no_change, max_iter)
        if verbose >= 2:
            print(f'm={m}, alpha={alpha}, rho={rho}, beta={beta}, theta={theta}, ksi={ksi}, q0={q0}')
            print('效益矩阵：')
            print(d)
            print(f'初始信息素：{tau0}')
            print()
        dt = 0
        curr_best_path_dist_list = []
        best_path_dist_list = []
        for n, record in finder:
            n += 1
            dt += record.dt
            curr_best_path_dist_list.append(record.cbpd)
            best_path_dist_list.append(record.bpd)
            if verbose >= 3:
                print(f'第{n + 1}次迭代：')
                if verbose >= 4:
                    print('信息素矩阵：')
                    print(record.tau)
                print(f'耗时：{record.idt}ms')
                print('该次迭代最优路径：', record.cbp)
                print('该次迭代最优路径长度：', record.cbpd)
                print('至今迭代最优路径：', record.bp)
                print('至今迭代最优路径长度：', record.bpd)
                print()

        best_path_dist = best_path_dist_list[-1]
        if verbose >= 1:
            print(f'贪心算法结果：{Cnn}')
            print(f'贪心算法耗时：{Cnn_delta_time}ms')
            print(f'蚁群算法最好结果：{best_path_dist}')
            print(f'蚁群算法耗时：{dt}ms')
            print(f'整数规划结果：{km}')
            print(f'整数规划耗时：{km_delta_time}ms')
            print(f'最坏情况：{worst_path_dist}')

        return curr_best_path_dist_list, best_path_dist_list, best_path_dist

    res_list = [test0(theta) for theta in thetas]
    if imgpath or draw:
        plt.rcParams['figure.figsize'] = (12, 9)
        plt.gcf().set_tight_layout(True)
        # 绘画贪婪算法结果
        plt.axhline(Cnn / worst_path_dist, label='贪婪算法')
        # 绘画最优解的结果
        plt.axhline(km / worst_path_dist, label='最优解')
        labels = ['蚁群系统'] * len(thetas)
        if len(thetas) > 1:
            for i, label in enumerate(labels):
                labels[i] = f'{label}（θ={thetas[i]}）'

        for (cbpdl, bpdl, bpd), label in zip(res_list, labels):
            # 当前迭代最优路径长度曲线
            plt.plot(np.divide(cbpdl, worst_path_dist), alpha=0.25)
            # 至今最优路径长度曲线
            plt.plot(np.divide(bpdl, worst_path_dist), label=label)
        plt.legend()
        if imgpath:
            plt.savefig(imgpath)
        if draw:
            plt.show()
        else:
            plt.close(plt.gcf())

    return [((Cnn - bpd) / worst_path_dist, (km - bpd) / worst_path_dist) for _, _, bpd in res_list]


def test_many(d, m, rho, alpha, beta, thetas, ksi, q0, n_iter_no_change=10, max_iter=200, verbose=0, draw_step=0,
              times=30, save_image=False):
    """
    :param d:
    :param m:
    :param rho:
    :param alpha:
    :param beta:
    :param thetas:
    :param ksi:
    :param q0:
    :param n_iter_no_change:
    :param max_iter:
    :param verbose:
    :param draw_step: 每多少轮测试输出一次图片
    :param times: 测试的次数
    :param save_image: 是否保存图片
    :return:
    """
    Cnn_delta_list = [[] for _ in enumerate(thetas)]
    km_delta_list = [[] for _ in enumerate(thetas)]
    for i in range(times):
        print(f'第{i}次测试：')
        dd = d() if callable(d) else d
        draw = i % draw_step == 0 if draw_step > 0 else False
        kwargs = {}
        if save_image:
            kwargs['imgpath'] = f'./data/{i + 1}.png'
        res = test(dd, m, rho, alpha, beta, thetas, ksi, q0, n_iter_no_change, max_iter, verbose, draw, **kwargs)
        for j, r in enumerate(res):
            Cnn_delta_list[j].append(r[0])
            km_delta_list[j].append(r[1])

    plt.rcParams['figure.figsize'] = (12, 9)
    plt.gcf().set_tight_layout(True)
    plt.axhline(0)
    for i, theta in enumerate(thetas):
        c = Cnn_delta_list[i]
        k = km_delta_list[i]
        ca = np.cumsum(c) / (np.arange(len(c)) + 1)
        ka = np.cumsum(k) / (np.arange(len(k)) + 1)
        plt.plot(ca, label=f'相较于贪婪算法的提升（θ={theta}）')
        plt.plot(ka, label=f'与最优解的差距（θ={theta}）')
    plt.legend()
    if save_image:
        plt.savefig('./data/performance.png')
    if draw_step != 0:
        plt.show()
    else:
        plt.close(plt.gcf())


def main():
    d = partial(random_cost, 20, 20, .05, .0)
    m = 10
    rho = 0.1
    alpha = 1
    beta = 2
    thetas = [1e1, 1e2, 1e3, inf]
    ksi = 0.1
    q0 = 0.9
    n_iter_no_change = 25
    max_iter = 2000
    verbose = 1
    draw_step = 0
    times = 300
    save_image = True
    return test_many(d, m, rho, alpha, beta, thetas, ksi, q0, n_iter_no_change, max_iter, verbose, draw_step, times,
                     save_image)


if __name__ == '__main__':
    main()
