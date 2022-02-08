FROM python:3.8

# 搭建环境
WORKDIR /app
COPY requirements.txt .
RUN mkdir -p ~/.pip/ \
    && echo "[global]\nindex-url = https://mirrors.aliyun.com/pypi/simple/" > ~/.pip/pip.conf \
    && pip install --upgrade pip \
    && pip install -r requirements.txt
VOLUME /app/run
STOPSIGNAL SIGKILL

# 复制项目文件
COPY . .

# 开放端口
EXPOSE 8001

# 启动
# CMD ["gunicorn", "-c", "config/gunicorn.conf.py", "app:app", "--access-logfile", "-"]
CMD ["python", "-u", "-X", "faulthandler", "-X", "utf8", "app.py"]