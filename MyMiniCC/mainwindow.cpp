#include "mainwindow.h"
#include "textoutputdialog.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "parse.h"
#include "symtab.h"
#include "QProcess"
#include "QDesktopServices"
#include <QTemporaryFile>
#include <sstream>
#include "codegenerator.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowTitle("MiniC IDE");

    ui->compileOutputDockWidget->setWindowTitle("编译结果");

    // 设置codeEdit的字体
    QFont font;
    font.setFamilies({"JetBrains Mono", "Consolas", "Courier New", "monospace"});
    font.setPointSize(10);
    ui->codeEdit->setFont(font);
    ui->codeEdit->setWordWrapMode(QTextOption::NoWrap);
}

MainWindow::~MainWindow() {
    delete ui;
    for (auto &svgFile: temFiles) {
        QFile(svgFile).remove();
    }
}

void MainWindow::on_actionnewFile_triggered() {
    ui->codeEdit->clear();
}

void MainWindow::on_actionopenFile_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件(*.*)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.exists() && file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            ui->codeEdit->clear();
            ui->codeEdit->append(file.readAll());
        }
    }
}

void MainWindow::on_actionsaveFile_triggered() {
    QString filename = QFileDialog::getSaveFileName(this, "选择文件", "", "所有文件(*.*)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        qDebug() << file.fileName();
        if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->codeEdit->toPlainText();
        }
        file.close();
    }
}

void MainWindow::on_actionanalyzeTokens_triggered() {
    ui->compileOutputBrowser->clear();
    ui->compileOutputDockWidget->setVisible(true);
    ui->compileOutputBrowser->append("开始扫描器分析...");
    std::wstringstream wss(ui->codeEdit->toPlainText().toStdWString());
    Scanner scanner(wss);
    vector<Token> tokens;
    for (Token token; (token = scanner.next_token()).type != TokenType::END; ) {
        tokens.emplace_back(token);
    }
    size_t maxTokenNameLen = 0;
    size_t maxEnglishLen = 0;
    size_t maxChineseLen = 0;
    for (auto &token: tokens) {
        maxTokenNameLen = max(maxTokenNameLen, token.value.size());
        maxEnglishLen = max(maxEnglishLen, wcslen(english_name(token.type)));
        maxChineseLen = max(maxChineseLen, wcslen(chinese_name(token.type)));
    }
    ui->compileOutputBrowser->append(QString("最长的Token长度为： %1").arg(maxTokenNameLen));
    ui->compileOutputBrowser->append(QString("最长的英文类型名长度为： %1").arg(maxEnglishLen));
    ui->compileOutputBrowser->append(QString("最长的中文类型名长度为： %1").arg(maxChineseLen));
    QString result;
    for(auto &token: tokens) {
        result.append(QString("%1 %2 %3\n")
                      .arg(token.value, maxTokenNameLen)
                      .arg(english_name(token.type), maxEnglishLen)
                      .arg(chinese_name(token.type), -maxChineseLen));
    }
    TextOutputDialog *dialog = new TextOutputDialog(this);
    dialog->setWindowTitle("扫描器分析结果");
    dialog->setResult(result);
    ui->compileOutputBrowser->append("完成！");
    dialog->show();
}

QString toString(const vector<Error> &errors) {
    QString result;
    for (auto error = errors.begin(); error != errors.end(); ++error) {
        if (error != errors.begin()) {
            result.append("\n");
        }
        wstring message = L"语法预期输入为    ";
        for (auto token = error->expected_tokens.begin(); token != error->expected_tokens.end(); ++token) {
            if (token != error->expected_tokens.begin()) {
                message.append(L"    ");
            }
            message.append(*token);
        }
        message.append(L"    ，但扫描到的输入为    ");
        message.append(error->token.value);
        message.append(L"    。");
        result.append(QString("在第%1行第%2列： %3")
                      .arg(error->token.lineno + 1)
                      .arg(error->token.colno + 1)
                      .arg(message));
    }
    return result;
}

void MainWindow::on_actionbuildTree_triggered() {
    ui->compileOutputBrowser->clear();
    ui->compileOutputDockWidget->setVisible(true);
    ui->compileOutputBrowser->append("开始解析语法树...");
    std::wstringstream wss(ui->codeEdit->toPlainText().toStdWString());
    Scanner scanner(wss);
    Parser parser;
    parser.parse(scanner);
    if (!parser.errors().empty()) {
        ui->compileOutputBrowser->append(toString(parser.errors()));
        ui->compileOutputBrowser->append("失败！");
    } else {
        wstring dot = to_dot(parser.result());
        QTemporaryFile dotFile;
        QTemporaryFile svgFile;
        dotFile.setAutoRemove(false);
        svgFile.setAutoRemove(false);
        dotFile.setFileTemplate("XXXXXX.dot");
        svgFile.setFileTemplate("XXXXXX.svg");
        if (dotFile.open() && svgFile.open()) {
            temFiles.emplace_back(dotFile.fileName());
            temFiles.emplace_back(svgFile.fileName());
            svgFile.close();
            ui->compileOutputBrowser->append(QString("创建临时文件： %1").arg(dotFile.fileName()));
            ui->compileOutputBrowser->append(QString("创建临时文件： %1").arg(svgFile.fileName()));
            QTextStream out(&dotFile);
            out << dot;
            dotFile.close();
            QString cmd = QString("\"C:\\Program Files\\Graphviz\\bin\\dot\" -Tsvg \"%1\" -o \"%2\"")
                    .arg(dotFile.fileName())
                    .arg(svgFile.fileName());
            ui->compileOutputBrowser->append(QString("执行命令： %1").arg(cmd));
            QProcess::execute(cmd);
            // 打开生成后的svg图片
            QDesktopServices::openUrl(QUrl(svgFile.fileName()));
        }
        ui->compileOutputBrowser->append("成功！");
    }
}

void MainWindow::on_actioncompileOutput_triggered() {
    ui->compileOutputDockWidget->setVisible(!ui->compileOutputDockWidget->isVisible());
}

QString toString(const vector<SymtabError> &errors) {
    static std::map<SymtabErrorType, const char *> templates = {
        {SymtabErrorType::NOT_ARROWED_TYPE, "无法使用指定类型进行声明： %1在位置 (%2, %3)。 "},
        {SymtabErrorType::NOT_FOUND, "无法找到符号定义： %1在位置 (%2, %3)。 "},
        {SymtabErrorType::WRONG_PARAM_SIZE, "函数调参数个数不匹配： %1在位置 (%2, %3)。请参考 (%4, %5) 函数定义。 "},
        {SymtabErrorType::REQUIRE_ARRAY, "需要传入数组： %1在位置 (%2, %3)。请参考 (%4, %5) 函数定义。 "},
        {SymtabErrorType::INCORRECT_USE, "错误使用： %1在位置 (%2, %3)。 声明在 (%4, %5)。 "},
        {SymtabErrorType::MULTI_DECLARATION, "多次定义变量： %1在位置 (%2, %3)。 第一次定义在 (%4, %5)。 "},
        {SymtabErrorType::TYPE_ERROR, "类型不匹配： %1在位置 (%2, %3)。 声明在 (%4, %5)。 "},
        {SymtabErrorType::ERROR_EXP_TYPE, "预期外的表达式返回值： %1在位置 (%2, %3)。 返回值应当为INT。 "},
        {SymtabErrorType::NO_COMPLETE_RETURN, "函数存在分支没有进行返回： %1在位置 (%2, %3)。 "},
        {SymtabErrorType::NO_MAIN, "没有定义main函数，无法找到入口。 "}
    };
    QString result;
    for (auto error = errors.begin(); error != errors.end(); ++error) {
        if (error != errors.begin()) {
            result.append("\n");
        }
        auto template0 = templates.at(error->type);
        result.append(QString(template0)
                      .arg(error->token.value.data())
                      .arg(error->token.lineno + 1)
                      .arg(error->token.colno + 1)
                      .arg(error->conflict.lineno + 1)
                      .arg(error->conflict.colno+ 1));
    }
    return result;
}

void MainWindow::on_actionbuildSymtab_triggered() {
    ui->compileOutputBrowser->clear();
    ui->compileOutputDockWidget->setVisible(true);
    ui->compileOutputBrowser->append("开始解析语法树...");
    std::wstringstream wss(ui->codeEdit->toPlainText().toStdWString());
    Scanner scanner(wss);
    Parser parser;
    parser.parse(scanner);
    if (!parser.errors().empty()) {
        ui->compileOutputBrowser->append(toString(parser.errors()));
        ui->compileOutputBrowser->append("失败！");
    } else {
        ui->compileOutputBrowser->append("成功！");
        ui->compileOutputBrowser->append("开始构建符号表...");
        ScopeSymtabs scopeSymtabs(parser.result());
        if (!scopeSymtabs.errors()->empty()) {
            ui->compileOutputBrowser->append(toString(*scopeSymtabs.errors()));
            ui->compileOutputBrowser->append("失败！");
        } else {
            ui->compileOutputBrowser->append("成功！");
        }
        TextOutputDialog *dialog = new TextOutputDialog(this);
        dialog->setWindowTitle("符号表");
        dialog->setResult(QString::fromStdWString(scopeSymtabs.to_string()));
        dialog->show();
    }
}

void MainWindow::on_actionCode_triggered() {
    ui->compileOutputBrowser->clear();
    ui->compileOutputDockWidget->setVisible(true);
    ui->compileOutputBrowser->append("开始解析语法树...");
    std::wstringstream wss(ui->codeEdit->toPlainText().toStdWString());
    Scanner scanner(wss);
    Parser parser;
    parser.parse(scanner);
    if (!parser.errors().empty()) {
        ui->compileOutputBrowser->append(toString(parser.errors()));
        ui->compileOutputBrowser->append("失败！");
    } else {
        ui->compileOutputBrowser->append("成功！");
        ui->compileOutputBrowser->append("开始构建符号表...");
        ScopeSymtabs scopeSymtabs(parser.result());
        if (!scopeSymtabs.errors()->empty()) {
            ui->compileOutputBrowser->append(toString(*scopeSymtabs.errors()));
            ui->compileOutputBrowser->append("失败！");
        } else {
            ui->compileOutputBrowser->append("成功！");
            ui->compileOutputBrowser->append("开始生成代码...");
            QTemporaryFile tmFile;
            tmFile.setAutoRemove(false);
            tmFile.setFileTemplate("XXXXXX.tm");
            if (tmFile.open()) {
                temFiles.emplace_back(tmFile.fileName());
                ui->compileOutputBrowser->append(QString("创建临时文件： %1").arg(tmFile.fileName()));
                QTextStream out(&tmFile);
                CodeGenerator codeGen(parser.result(), scopeSymtabs);
                out << codeGen.to_string();
                tmFile.close();
            }
            QString cmd = QString("\"%1\" \"%2\"")
                    .arg(QCoreApplication::applicationDirPath().append("/tm.exe"))
                    .arg(tmFile.fileName());
            ui->compileOutputBrowser->append("成功");
            TextOutputDialog *dialog = new TextOutputDialog(this);
            dialog->setWindowTitle("运行");
            dialog->setResult(QString("请复制以下命令到cmd执行：\n").append(cmd));
            dialog->show();
        }
    }
}
