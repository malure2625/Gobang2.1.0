#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ChessPlay.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	QFont font = ui->btnAbout->font();
	ui->btnAbout->setFont(DPI_Font(font));
	font = ui->btnComputer->font();
	ui->btnComputer->setFont(DPI_Font(font));
	font = ui->label_2->font();;
	ui->label_2->setFont(DPI_Font(font));

    this->setFixedSize(850,640);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnAbout_clicked()
{
    QMessageBox::information(this,QStringLiteral("    "),QStringLiteral("-----本软件由malu制作------"),QMessageBox::NoButton);
}

void MainWindow::on_btnComputer_clicked()
{
    int ret;
    this->close();
    ChessPlay* pvc=new ChessPlay;
    pvc->show();
	pvc->putinAdversaryName();
    pvc->judgeWhoFirst();
}

QFont MainWindow::DPI_Font(QFont font)
{
	int fontsize = font.pixelSize();
	if (fontsize == -1)
	{
		fontsize = font.pointSize();
		if (fontsize == -1)
		{
			qreal sizef = font.pointSizeF();
			font.setPointSizeF(sizef);
		}
		else
		{
			font.setPixelSize(fontsize * 90 / 72); // 设置字体大小为像素大小，而不是以磅为单位,
		}
	}
	else
	{
		font.setPixelSize(fontsize);
	}
	return font;
}