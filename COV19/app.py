import logging
from flasgger import Swagger
from flask import Flask, request
from flask.wrappers import Response
from os import environ
import pandas as pd
from threading import Thread
from time import sleep
import re
import torch
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from typing import Dict, List

TIPS = 25
logging.basicConfig(level=TIPS)


class Loader(Thread):
    """加载器，使用线程来做耗时的加载工作。"""

    def run(self):
        logging.log(TIPS, "开始加载模型。")
        # 加载 pre-trained model/tokenizer
        Job.tokenizer = AutoTokenizer.from_pretrained(
            "distilbert-base-uncased-finetuned-sst-2-english"
        )
        Job.bert = AutoModelForSequenceClassification.from_pretrained(
            "distilbert-base-uncased-finetuned-sst-2-english", return_dict=True
        )
        # 尝试使用GPU
        Job.device = "cuda:0" if torch.cuda.is_available() else "cpu"
        Job.bert = Job.bert.to(Job.device)
        # 加载完成
        logging.log(TIPS, "模型加载完成。")
        Job.loadding = False


class Job(Thread):
    """作业，使用预训练模型对输入数据打分并保存结果，可以获取进度。"""

    # 模型
    loadding = True
    tokenizer = None
    bert = None
    device = None

    def __init__(self, text_list: List[str]):
        Thread.__init__(self)
        self.text_list = text_list
        self.progress = 0
        self.neg_list = []
        self.pos_list = []

    def run(self):
        while self.loadding:
            # 等待加载完成
            sleep(1)

        text_list_len = len(self.text_list)
        for i, text in enumerate(self.text_list):
            self.progress = i / text_list_len
            [[neg, pos]] = self.bert(
                self.tokenizer(text, return_tensors="pt").to(self.device)["input_ids"]
            ).logits
            self.neg_list.append(torch.sigmoid(neg).item())
            self.pos_list.append(torch.sigmoid(pos).item())
        # TODO: 其它处理
        # 设置任务完成
        self.progress = 1



class Jobs:
    def __init__(self):
        self.count = 0
        self.m: Dict[int, Job] = {}

    def new(self, text_list):
        job_id = self.count
        self.count += 1
        job = Job(text_list)
        self.m[job_id] = job
        job.start()
        return job_id


app = Flask(__name__)
swagger = Swagger(app)
loader = Loader()
jobs = Jobs()


def rm_char(s):
    s = str(s)
    s += " "
    s = re.sub(r"@.*?\s+", " ", s)
    s = re.sub(r"#.*?\s+", " ", s)
    s = re.sub(r"http.*?\s+", " ", s)
    s = re.sub(r"[^a-zA-Z0-9]+", " ", s)
    return s


@app.route("/text_prediction", methods=["POST"])
def predict_text():
    """
    预测一个句子。
    ---
    parameters:
      - name: text
        in: formData
        type: string
        required: true
    responses:
      201:
        description: 创建作业成功，返回作业号。
      400:
        description: 表单中没有text字段。
      default:
        description: 未知错误。
    """
    text = request.form.get("text")
    if text is None:
        return {"message": "表单中需要含有text字段。"}, 400
    text = rm_char(text)
    return {"id": jobs.new([text])}, 201


@app.route("/csv_prediction", methods=["POST"])
def predict_csv():
    """
    预测csv。
    ---
    parameters:
      - name: csv
        in: formData
        type: file
        required: true
    responses:
      201:
        description: 创建作业成功，返回作业号。
      400:
        description: 有三种情况：<br /> 1. 表单中没有csv字段。<br />2. 上传的不是一个csv文件。<br />3. csv文件中没有text字段。
      default:
        description: 未知错误。
    """
    csv = request.files.get("csv")
    if csv is None:
        return {"message": "表单中需要用csv字段来上传csv文件。"}, 400
    try:
        df = pd.read_csv(csv)
        df.text = df.text.map(rm_char)
        return {"id": jobs.new(df.text.values)}, 201
    except pd.errors.ParserError:
        return {"message": "不是一个csv文件。"}, 400
    except AttributeError:
        return {"message": "csv文件中需要有text字段。"}, 400


@app.route("/prediction_result/<int:id>")
def get_result(id):
    """
    获取预测结果。
    ---
    parameters:
      - name: id
        in: path
        schema:
            type: integer
            default: 0
        required: true
    responses:
      200:
        description: 作业已完成，返回预测输出的两个数组。
      202:
        description: 作业仍在运行，以小数的形式返回当前进度。
      404:
        description: 未找到该作业。
      default:
        description: 未知错误。
    """
    job = jobs.m.get(id)
    if job is None:
        # 不存在的作业
        return Response(status=404)
    if job.progress < 1:
        # 处理中
        return {"progress": job.progress}, 202
    else:
        # 处理完成
        jobs.m.pop(id)
        return {"neg_list": job.neg_list, "pos_list": job.pos_list}, 200

dev = environ.get("FLASK_ENV") == "development"
# 加载模型，且只加载一次
if not dev or environ.get("WERKZEUG_RUN_MAIN") == "true":
    loader.start()

if __name__ == "__main__":
    # 开发模式下，debug=True
    app.run(debug=dev, host="0.0.0.0")
