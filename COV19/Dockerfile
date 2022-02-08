FROM pytorch/pytorch:1.10.0-cuda11.3-cudnn8-runtime

# 运行环境
COPY requirements.txt .
RUN python -m pip install -r requirements.txt -i https://mirrors.aliyun.com/pypi/simple/

# 开发环境
ARG DEV
ARG PKG="git"
COPY requirements-dev.txt .
RUN if [ $DEV ]; then \
    sed -i -e 's/archive.ubuntu/mirrors.aliyun/g' -e 's/security.ubuntu/mirrors.aliyun/g' /etc/apt/sources.list && \
    apt-get update && (apt-get install -y ${PKG} || apt-get install -y ${PKG} --fix-missing) && \
    python -m pip install -r requirements-dev.txt -i https://mirrors.aliyun.com/pypi/simple/ \
    ; fi

# 复制项目
WORKDIR /app
COPY . /app
RUN rm requirements.txt requirements-dev.txt

CMD ["flask", "run", "--host=0.0.0.0"]
