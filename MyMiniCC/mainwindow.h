#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void on_actionnewFile_triggered();

    void on_actionopenFile_triggered();

    void on_actionsaveFile_triggered();

    void on_actionanalyzeTokens_triggered();

    void on_actionbuildTree_triggered();

    void on_actioncompileOutput_triggered();

    void on_actionbuildSymtab_triggered();

    void on_actionCode_triggered();

private:
    Ui::MainWindow *ui;
    std::vector<QString> temFiles;
};
#endif // MAINWINDOW_H
