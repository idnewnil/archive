"use strict";

let guide = {
    sprite: {
        InAtlas: "atlas.png",                                       // 在?地图集
        locationBy: "atlas.csv"                                     // 通过?进行定位
    },
    canvas: {
        id: "cvs"                                                   // 画布ID
    },
    gameStateEnum: {init: 0, ready: 1, playing: 2, gameOver: 3},    // 游戏状态
    physics: {
        gravity: 1.5                                                // 重力加速度
    },
    background: {
        style: "bg_day",                                            // 背景风格
        dye: {
            color: "white"                                          // 染色
        }
    },
    land: {
        style: "land",                                              // 地面风格
        dye: {
            color: "black"                                          // 染色
        },
        y: 400,                                                     // 地面高度
        xSpeed: 4                                                   // 后退速度
    },
    bird: {
        style: ["bird0_0", "bird0_1", "bird0_2", "bird0_1"],        // 小鸟风格
        margin: {                                                   // 小鸟sprite附近会有一层比较厚的透明像素，margin指出其宽度
            top: 12,
            bottom: 12,
            left: 6,
            right: 8
        },
        dye: {
            color: "orange"                                         // 染色
        },
        float: {                                                    // 浮动：用于配置最开始的动画
            period: 50,                                             // 周期
            amplitude: 10                                           // 振幅
        },
        x: 69,                                                      // 初始的X位置
        initialY: 236,                                              // 初始的Y位置
        jumpSpeed: -12,                                             // 点击屏幕立即获得的Y方向的速度
        maxFallSpeed: 15,                                           // 最大掉落速度
        wingPerFrame: 0.2                                           // 煽动翅膀的频率
    },
    pipe: {
        topStyle: "pipe_down",                                      // 上管道风格
        bottomStyle: "pipe_up",                                     // 下管道风格
        margin: {                                                   // 管道sprite左右两侧会有一层透明像素，margin指出其宽度，这个
            left: 4,
            right: 4
        },
        dye: {
            color: "darkgreen"                                      // 染色
        },
        gapBetweenUpAndBottom: 120,                                 // 管道上下空隙
        distanceBetweenTwoPipes: 172,                               // 两个管道间的距离
        initialX: 358,                                              // 生成的新管道的X位置
        boundary: 50,                                               // 上下管道的最短长度
        xSpeed: -4                                                  // 后退速度
    },
    render: {
        defaultFps: 40
    }
};

// 加载sprite
let sprite = new function () {
    // 加载图片资源
    this.atlas = new Image();
    this.location = {};
    this.loadOver = false;      // 是否加载完成

    this.atlas.addEventListener("load", function () {
        let req = new XMLHttpRequest();
        req.open("GET", guide.sprite.locationBy);
        req.addEventListener("load", function () {
            req.responseText.split("\n").forEach(function (line) {
                let values = line.split(" ");
                sprite.location[values[0]] = {
                    x: Math.round(parseFloat(values[3]) * sprite.atlas.width),
                    y: Math.round(parseFloat(values[4]) * sprite.atlas.height),
                    w: Math.round(parseInt(values[1], 10)),
                    h: Math.round(parseInt(values[2], 10))
                }
            });
            sprite.loadOver = true;
        });
        req.send();
    });
    this.atlas.src = guide.sprite.InAtlas;
};

// 获取画布
let canvas;
window.addEventListener("load", function () {
    canvas = document.getElementById(guide.canvas.id);
    canvas.context = canvas.getContext("2d");
    canvas.context.dye = function (x, y, w, h, color) {
        let oldColor = canvas.context.strokeStyle;
        canvas.context.fillStyle = color || oldColor;
        canvas.context.fillRect(x, y, w, h);
        canvas.context.fillStyle = oldColor;
    };
});

let component = {
    Background: function () {
        this.draw = function (dye) {
            let s = sprite.location[guide.background.style];
            let d = {x: 0, y: 0, w: s.w, h: s.h};
            canvas.context.drawImage(sprite.atlas, s.x, s.y, s.w, s.h, d.x, d.x, d.w, d.h);
            !dye ? canvas.context.drawImage(sprite.atlas, s.x, s.y, s.w, s.h, d.x, d.x, d.w, d.h)
                : canvas.context.dye(d.x, d.y, d.w, d.h, guide.background.dye.color);
            return d;
        };
        this.nextFrame = function () {
        };
    },
    Land: function () {
        this.offset = 0;
        this.draw = function (dye) {
            let s = sprite.location[guide.land.style];
            let d = {x: 0, y: guide.land.y, w: s.w, h: s.h};
            let sw1 = ((s.x + this.offset) % s.w + s.w) % s.w || 1;
            let sw2 = s.w - sw1 || 1;
            !dye && canvas.context.drawImage(sprite.atlas, s.x + sw1, s.y, sw2, s.h, d.x, d.y, sw2 / s.w * d.w, d.h);
            !dye && canvas.context.drawImage(sprite.atlas, s.x, s.y, sw1, s.h, d.x + sw2 / s.w * d.w, d.y, sw1 / s.w * d.w, d.h);
            dye && canvas.context.dye(d.x, d.y, d.w, d.h, guide.land.dye.color)
            return d;
        };
        this.nextFrame = function () {
            let lc = sprite.location[guide.land.style];
            this.offset = ((this.offset + guide.land.xSpeed) % lc.w + lc.w) % lc.w;  // 范围在[0, lc.w)
        };
    },
    Pipe: function () {
        this.x = guide.pipe.initialX;
        this.topHeight = guide.pipe.boundary + Math.random() * (guide.land.y - guide.pipe.gapBetweenUpAndBottom - 2 * guide.pipe.boundary);
        this.bottomY = this.topHeight + guide.pipe.gapBetweenUpAndBottom;
        this.draw = function (dye) {
            let ts = sprite.location[guide.pipe.topStyle];
            let bs = sprite.location[guide.pipe.bottomStyle];
            ts = {x: ts.x, y: ts.y + ts.h - this.topHeight, w: ts.w, h: this.topHeight};
            bs = {x: bs.x, y: bs.y, w: bs.w, h: guide.land.y - this.bottomY};
            let td = {x: this.x, y: 0, w: ts.w, h: this.topHeight};
            let bd = {x: this.x, y: this.bottomY, w: bs.w, h: bs.h}
            !dye && canvas.context.drawImage(sprite.atlas, ts.x, ts.y + ts.h - this.topHeight, ts.w, this.topHeight, this.x, 0, ts.w, this.topHeight);
            !dye && canvas.context.drawImage(sprite.atlas, bs.x, bs.y, bs.w, bs.h, this.x, this.bottomY, bs.w, bs.h);
            td.x += guide.pipe.margin.left;
            td.w -= guide.pipe.margin.left + guide.pipe.margin.right;
            bd.x += guide.pipe.margin.left;
            bd.w -= guide.pipe.margin.left + guide.pipe.margin.right;
            dye && canvas.context.dye(td.x, td.y, td.w, td.h, guide.pipe.dye.color);
            dye && canvas.context.dye(bd.x, bd.y, bd.w, bd.h, guide.pipe.dye.color);
            return {top: td, bottom: bd};
        };
        this.nextFrame = function () {
            this.x += guide.pipe.xSpeed;
        };
        this.isVisible = function () {
            return this.x + sprite.location[guide.pipe.bottomStyle].w > 0;
        };
    },
    Bird: function () {
        this.float = true;
        this.frameIteration = 0;
        this.y = guide.bird.initialY;
        this.ySpeed = guide.bird.jumpSpeed;
        this.jump = function () {
            this.float = false;
            this.ySpeed = guide.bird.jumpSpeed;
        };
        this.draw = function (dye) {
            let s = sprite.location[guide.bird.style[~~(this.frameIteration * guide.bird.wingPerFrame) % guide.bird.style.length]];
            let d = {x: guide.bird.x, y: this.y, w: s.w, h: s.h};
            !dye && canvas.context.drawImage(sprite.atlas, s.x, s.y, s.w, s.h, d.x, d.y, d.w, d.h);
            d.x += guide.bird.margin.left;
            d.w -= guide.bird.margin.left + guide.bird.margin.right;
            d.y += guide.bird.margin.top;
            d.h -= guide.bird.margin.top + guide.bird.margin.bottom;
            dye && canvas.context.dye(d.x, d.y, d.w, d.h, guide.bird.dye.color);
            return d;
        };
        this.nextFrame = function () {
            if (this.float) {
                this.y = guide.bird.initialY
                    + Math.sin(this.frameIteration / guide.bird.float.period * 2 * Math.PI) * guide.bird.float.amplitude;
            } else {
                this.y = Math.max(-sprite.location[guide.bird.style[0]].h * 0.67,
                    this.y + this.ySpeed + guide.physics.gravity / 2);  // 用max防止小鸟跳出画面
                this.ySpeed = Math.min(this.ySpeed + guide.physics.gravity, guide.bird.maxFallSpeed); // min限制小鸟最大掉落速度
            }
            ++this.frameIteration;
        };
    }
};

function isRectIntersect(a, b) {
    // 对立事件：两矩形不相交
    return !(a.x + a.w < b.x || a.y + a.h < b.y || b.x + b.w < a.x || b.y + b.h < a.y);
}

let ql = {
    argsChange: true,
    enable: false,
    alpha: 0.6, // 学习率
    gamma: 0.8, // 衰减系数
    blur: 15,   // 模糊化坐标，有效降低状态空间的大小
    reword: {   // 奖励
        alive: 1,
        dead: -100
    },
    runtime: {
        Q: null,
        A: null,
        S: null
    }
};

function updateQTable(Q, S, S_, A, R) {
    if (S && S_ && A in [0, 1] && S in Q && S_ in Q)
        Q[S][A] = (1 - ql.alpha) * Q[S][A] + ql.alpha * (R + ql.gamma * (Q[S_][0] > Q[S_][1] ? Q[S_][0] : Q[S_][1]));
}

let game = {
    background: null,
    land: null,
    pipeList: [],
    lastPassPipe: null,
    bird: null,
    state: guide.gameStateEnum.init,
    dye: false,
    animation: 0,
    frameIteration: [],
    maxFps: 40,
    round: 0,
    score: 0,
    maxScore: 0
}

function loopAnd(then) {
    if (!sprite.loadOver) {
        setTimeout(then, 333); // 资源未加载完成则继续等待
        return;
    }

    if (game.state === guide.gameStateEnum.init) {
        game.maxScore = game.score > game.maxScore ? game.score : game.maxScore;
        game.score = 0;
        if (ql.argsChange) {
            ql.runtime.Q = {}
            ql.argsChange = false;
        }
        ql.runtime.S = null;
        ql.runtime.A = null;
        canvas.width = sprite.location[guide.background.style].w;
        canvas.height = sprite.location[guide.background.style].h;
        game.frameIteration = [0];
        game.background = new component.Background();
        game.land = new component.Land();
        game.pipeList = [new component.Pipe()];
        game.bird = new component.Bird();
        game.state = guide.gameStateEnum.ready;
        setTimeout(then, 0);
    } else if (game.state === guide.gameStateEnum.ready) {
        if (ql.enable) {
            canvas.onmousedown = null;
            game.state = guide.gameStateEnum.playing;
            game.bird.jump();
        } else {
            canvas.onmousedown = function () {
                game.state = guide.gameStateEnum.playing;
                game.bird.jump();
            };
        }

        game.background.draw(game.dye);
        game.land.draw(game.dye);
        game.bird.draw(game.dye);
        game.background.nextFrame();
        game.land.nextFrame();
        game.bird.nextFrame();
        setTimeout(then, 1000 / game.maxFps);
    } else if (game.state === guide.gameStateEnum.playing) {
        canvas.onmousedown = ql.enable ? null : () => game.bird.jump();

        game.pipeList = game.pipeList.filter(pipe => pipe.isVisible());
        if (guide.pipe.initialX - game.pipeList[game.pipeList.length - 1].x >= guide.pipe.distanceBetweenTwoPipes) {
            game.pipeList.push(new component.Pipe());
        }

        let bgr = game.background.draw(game.dye);
        let lr = game.land.draw(game.dye);
        let pr = [];
        game.pipeList.forEach(pipe => pr.push(pipe.draw(game.dye)));
        let br = game.bird.draw(game.dye);
        if (pr[0].top.x + pr[0].top.w < br.x) {
            if (game.lastPassPipe !== game.pipeList[0]) {
                game.lastPassPipe = game.pipeList[0];
                ++game.score;
            }
            pr = pr.slice(1);
        }

        if (game.dye)
            canvas.context.dye(pr[0].top.x, pr[0].top.y, pr[0].top.w, pr[0].top.h, "yellow");

        let Q, S, A, S_;
        if (ql.enable) {
            Q = ql.runtime.Q;
            S = ql.runtime.S;
            A = ql.runtime.A;
            S_ = [Math.floor((pr[0].bottom.x) / ql.blur), Math.floor((pr[0].bottom.y - br.y) / ql.blur)].join(',');
            if (!(S_ in Q)) Q[S_] = [0, 0];
        }

        if (isRectIntersect(br, lr) || pr.some(pri => isRectIntersect(br, pri.top) || isRectIntersect(br, pri.bottom))) {
            game.state = guide.gameStateEnum.gameOver;
            if (ql.enable) {
                updateQTable(Q, S, S_, A, ql.reword.dead);
            }
        } else {
            if (ql.enable) {
                updateQTable(Q, S, S_, A, ql.reword.alive);
                let A_ = Q[S_][0] >= Q[S_][1] ? 0 : 1;
                if (A_ === 1) game.bird.jump();
                ql.runtime.A = A_;
                ql.runtime.S = S_;
            }
            game.background.nextFrame();
            game.land.nextFrame();
            game.pipeList.forEach(pipe => pipe.nextFrame());
            game.bird.nextFrame();
        }

        setTimeout(then, 1000 / game.maxFps);
    } else if (game.state === guide.gameStateEnum.gameOver) {
        ql.enable
            ? game.state = guide.gameStateEnum.init
            : canvas.onmousedown = () => game.state = guide.gameStateEnum.init;
        game.background.draw(game.dye);
        game.land.draw(game.dye);
        game.pipeList.forEach(pipe => pipe.draw(game.dye));
        game.bird.draw(game.dye);
        setTimeout(then, 1000 / game.maxFps);
    } else {
        throw new Error();
    }
}

//episode
