#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <yarp/os/all.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::testeFn(){
//    std::cout << " teste";
    qDebug() << " teste";
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() <<"teste";
}
