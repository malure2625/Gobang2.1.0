#include "ChessPlay.h"
#include "ui_ChessPlay.h"
#include "QMessageBox"
#include "QInputDialog"
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include "MainWindow.h"
#include <QTime>
#include <QThread>
#include <QTimer>
#include <fstream>
#include <QtDebug>
#include <QDateTime>
#include <QtCore/QCoreApplication>
#include <iostream>
#include "GameThread.h"

using namespace std;


ChessPlay::ChessPlay(QWidget *parent) :
	QMainWindow(parent), game(),
	ui(new Ui::ChessPlay)
{
	ui->setupUi(this);
	QFont font = ui->label->font();
	ui->label->setFont(MainWindow::DPI_Font(font));
	font = ui->label_2->font();
	ui->label_2->setFont(MainWindow::DPI_Font(font));
	font = ui->label_3->font();
	ui->label_3->setFont(MainWindow::DPI_Font(font));
	font = ui->dadianN->font();
	ui->dadianN->setFont(MainWindow::DPI_Font(font));
	font = ui->blackTime->font();
	ui->blackTime->setFont(MainWindow::DPI_Font(font));
	font = ui->whiteTime->font();
	ui->whiteTime->setFont(MainWindow::DPI_Font(font));
	font = ui->AttactMessage->font();
	ui->AttactMessage->setFont(MainWindow::DPI_Font(font));
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
			game.chess[i][j].seq = 255;
	mouseflag = false;
	whiteTimes = 900;
	blackTimes = 900;
	this->setFixedSize(850, 660);
	this->setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, QColor("#B1723C"));
	this->setPalette(palette);
	centralWidget()->setMouseTracking(true);
	setMouseTracking(true);
	gameThread = new GameThread(game);
	btimerId = new QTimer(this);
	wtimerId = new QTimer(this);
	/*gameThread = new myThread();*/
	connect(btimerId, SIGNAL(timeout()), this, SLOT(btimerUpdate()));
	connect(wtimerId, SIGNAL(timeout()), this, SLOT(wtimerUpdate()));
	connect(gameThread, SIGNAL(goon()), this, SLOT(computerPlaychessGoon()));
	/*connect(gameThread, SIGNAL(myThread::mySignal()), this, SLOT(AISlot()));*/
}

void ChessPlay::judgeN()
{
	bool ok;
	int N;
	while (true)
	{
		N = QInputDialog::getInt(this, QStringLiteral("打点数量选择"), QStringLiteral("请输入打点数量："), QLineEdit::Normal, 0, 100, 1);
		if (N)
			break;
	}
	game.dadianN = game.dadiann = N;
	ui->dadianN->setText(QString::number(game.dadianN));
}

void ChessPlay::putinAdversaryName()
{
	bool ok;

	QString text = QInputDialog::getText(this, QStringLiteral("队伍信息确认"), QStringLiteral("请输入对方队伍名："), QLineEdit::Normal, adversaryName, &ok);

	if (!ok || text.isEmpty())
		adversaryName = QStringLiteral("无");
	else
		adversaryName = text;
}


void ChessPlay::judgeWhoFirst()
{
	QMessageBox box(QMessageBox::Question, QStringLiteral("选择"), QStringLiteral("由谁先手"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	box.setButtonText(QMessageBox::Yes, QString(QStringLiteral("电脑后手")));
	box.setButtonText(QMessageBox::No, QString(QStringLiteral("电脑先手")));
	if (box.exec() == QMessageBox::Yes)
		peopleFirst();
	else
		computerFirst();
}

void ChessPlay::peopleFirst()
{
	game.cComputer = WHITE_CHESS;
	game.cPeople = BLACK_CHESS;
	judgeN();
	mouseflag = true;
}

void ChessPlay::computerFirst()
{
	game.cComputer = BLACK_CHESS;
	game.cPeople = WHITE_CHESS;
	game.LayOut();
	ui->dadianN->setText(QString::number(game.dadianN));
	peopleExchange();
}

void ChessPlay::peopleExchange()
{
	QMessageBox box(QMessageBox::Question, QStringLiteral("三手可交换"), QStringLiteral("是否换手"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	box.setButtonText(QMessageBox::Yes, QString(QStringLiteral("是")));
	box.setButtonText(QMessageBox::No, QString(QStringLiteral("否")));
	if (box.exec() == QMessageBox::Yes)
	{
		game.cComputer = WHITE_CHESS;
		game.cPeople = BLACK_CHESS;
		timeUpdate();
		computerPlaychess();
	}
	else
		timeUpdate();
	mouseflag = true;
}

bool ChessPlay::computerExchange()
{
	if (game.change())
	{
		QMessageBox box(QMessageBox::Question, QStringLiteral("三手可交换"), QStringLiteral("电脑选择：否"));
		box.exec();
		return false;
	}
	else
	{
		QMessageBox box(QMessageBox::Question, QStringLiteral("三手可交换"), QStringLiteral("电脑选择：是"));
		box.exec();
		game.cComputer = BLACK_CHESS;
		game.cPeople = WHITE_CHESS;
		timeUpdate();
		return true;
	}
}

void ChessPlay::Nda(int x, int y)
{
	game.currentX = x;
	game.currentY = y;
	for (int i = 0; i < game.dadiann; i++)
		game.backChess();
	game.chess[x][y].flag = 4;
	game.chess[x][y].seq = 4;
	game.player++;
	game.chessPoint.push_back(QPoint(x, y));
	game.dafou = true;
	update();
}

void ChessPlay::mouseReleaseEvent(QMouseEvent* event)
{
	int x, y;
	if (mouseflag)
	{
		x = (event->y() - 40) / 40;
		y = (event->x() - 20) / 40;
		if (game.player == 5 && !game.dafou)
			Nda(x, y);
		else if (event->x() >= 20 && event->x() <= 620 && event->y() >= 40 && event->y() <= 640)
		{
			if (JudgeIsExist(x, y))
			{
				game.AIkongzhi = true;
				while (game.waitThread);
				game.AIkongzhi = false;
				if (peoplePlaychess(x, y))
					return;
				computerPlaychess();/*
				tiaoshizhuanyong(x, y);*/
			}
			else
				QMessageBox::information(this, QStringLiteral("注意"), QStringLiteral("已存在棋子，请重下！"), QMessageBox::Ok);
		}
		else
			QMessageBox::information(this, QStringLiteral("注意"), QStringLiteral("不在棋盘内，请重下！"), QMessageBox::Ok);
		update();
	}
}

//void ChessPlay::myThread::run()
//{
//	METHOD_PROLOGUE(ChessPlay, myThread);
//	pThis->game.AI();
//	emit mySignal();
//}
//
//void ChessPlay::AISlot()
//{
//	QPoint point = game.computerPutdown(game.comX, game.comY);
//	game.currentX = game.chessPoint.last().x() = point.x();
//	game.currentY = game.chessPoint.last().y() = point.y();
//	if ((game.player - 1) % 2)
//	{
//		wtimerId->stop();
//		btimerId->start(1000);
//	}
//	else
//	{
//		btimerId->stop();
//		wtimerId->start(1000);
//	}
//	update();
//	mouseflag = true;
//	if (game.ifWin(game.chessPoint.last().x(), game.chessPoint.last().y()) && game.player != 5)
//	{
//		if ((game.player - 1) % 2)
//			QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("白棋获胜"));
//		else
//			QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("黑棋获胜"));
//		btimerId->stop();
//		wtimerId->stop();
//		mouseflag = false;
//		return;
//	}
//}

void ChessPlay::btimerUpdate()
{
	QString timeString = QString("%1-%2").arg(whiteTimes / 60, 2, 10, QChar('0')).arg(whiteTimes % 60, 2, 10, QChar('0'));
	ui->whiteTime->setText(timeString);
	if (!whiteTimes)
	{
		QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("黑棋超时,白棋获胜!"));
		whiteTimes = 900;
	}
	else
		whiteTimes--;
	update();
}
void ChessPlay::wtimerUpdate()
{
	QString timeString = QString("%1-%2").arg(blackTimes / 60, 2, 10, QChar('0')).arg(blackTimes % 60, 2, 10, QChar('0'));
	ui->blackTime->setText(timeString);
	if (!blackTimes)
	{
		QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("白棋超时,黑棋获胜!"));
		blackTimes = 900;
	}
	else
		blackTimes--;
	update();
}


void ChessPlay::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	QPen pen = painter.pen();
	pen.setColor(QColor("#8D5822"));
	pen.setWidth(7);
	painter.setPen(pen);
	QBrush brush;
	brush.setColor(QColor("#EEC085"));
	brush.setStyle(Qt::SolidPattern);
	painter.setBrush(brush);
	painter.drawRect(20, 40, 600, 600);
	pen.setColor(Qt::black);
	pen.setWidth(1);
	painter.setPen(pen);
	for (int i = 0; i<15; i++)
	{
		painter.drawLine(40 + i * 40, 60, 40 + i * 40, 620);//纵线
		QFont font("Arial", 8, QFont::Bold, false);
		painter.setFont(font);
		painter.drawText(35 + i * 40, 660, QString('A' + i));
		painter.drawLine(40, 60 + i * 40, 600, 60 + i * 40);//横线
		painter.drawText(0, 67 + i * 40, QString::number(15 - i));
	}
	if (game.message.type == 0)
		ui->AttactMessage->setText(QStringLiteral(" "));
	else if (game.message.type == 1)
		ui->AttactMessage->setText(QStringLiteral("正在进行vcf攻击:\n") + QStringLiteral("深度：") + QString::number(game.message.depth) + QStringLiteral("\n节点数：") + QString::number(game.message.cnt));
	else if (game.message.type == 2)
		ui->AttactMessage->setText(QStringLiteral("正在进行vcf防御:\n") + QStringLiteral("深度：") + QString::number(game.message.depth) + QStringLiteral("\n节点数：") + QString::number(game.message.cnt));
	else if (game.message.type == 3)
		ui->AttactMessage->setText(QStringLiteral("正在进行vcf和vct\n联合防御:\n") + QStringLiteral("深度：") + QString::number(game.message.depth) + QStringLiteral("\n节点数：") + QString::number(game.message.cnt));
	else if (game.message.type == 4)
		ui->AttactMessage->setText(QStringLiteral("正在进行vcf和vct\n联合攻击:\n") + QStringLiteral("深度：") + QString::number(game.message.depth) + QStringLiteral("\n节点数：") + QString::number(game.message.cnt));
	else if (game.message.type == 5)
		ui->AttactMessage->setText(QStringLiteral("正在进行迭代加深:\n") + QStringLiteral("深度：") + QString::number(game.message.depth));
	else if (game.message.type == 33)
		ui->AttactMessage->setText(QStringLiteral("computer play is true"));
	/*ui->AttactMessage->setWordWrap(true);
	ui->AttactMessage->setAlignment(Qt::AlignTop);*/
	brush.setColor(Qt::black);
	painter.setBrush(brush);
	painter.drawRect(155, 175, 10, 10);
	painter.drawRect(475, 175, 10, 10);
	painter.drawRect(155, 495, 10, 10);
	painter.drawRect(475, 495, 10, 10);
	painter.drawRect(315, 335, 10, 10);
	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
		{
			if (game.chess[i][j].flag == 4)
			{
				brush.setColor(Qt::black);
				painter.setBrush(brush);
				painter.drawRect((j + 1) * 40 - 9, (i + 1) * 40 + 20 - 9, 18, 18);
			}
			else if (game.chess[i][j].flag == 5)
			{
				brush.setColor(Qt::white);
				painter.setPen(Qt::NoPen);
				painter.setBrush(brush);
				painter.drawRect((j + 1) * 40 - 9, (i + 1) * 40 + 20 - 9, 18, 18);
			}
		}
	for (int i = 0;; i++)
	{
		if (game.player != 5 || game.dafou)
			if (i >= game.player)
				break;
			else;
		else
			if (i >= game.player + game.dadiann - 1)
				break;
		if (game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].flag == 4)
		{
			brush.setColor(Qt::black);
			painter.setBrush(brush);
			painter.drawEllipse(QPoint((game.chessPoint[i].y() + 1) * 40, (game.chessPoint[i].x() + 1) * 40 + 20), 18, 18);
			QRectF ff((game.chessPoint[i].y() + 1) * 40 - 18, (game.chessPoint[i].x() + 1) * 40 + 2, 36, 36);
			painter.setPen(QColor(Qt::red));
			if (game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq != game.player - 1 && game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq != 255)
			{
				painter.drawText(ff, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq + 1));
			}
			painter.setPen(Qt::NoPen);
		}
		else if (game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].flag == 5)
		{
			brush.setColor(Qt::white);
			painter.setPen(Qt::NoPen);
			painter.setBrush(brush);
			painter.drawEllipse(QPoint((game.chessPoint[i].y() + 1) * 40, (game.chessPoint[i].x() + 1) * 40 + 20), 18, 18);
			QRectF ff((game.chessPoint[i].y() + 1) * 40 - 18, (game.chessPoint[i].x() + 1) * 40 + 2, 36, 36);
			painter.setPen(QColor(Qt::red));
			if (game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq != game.player - 1 && game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq != 255)
			{
				painter.drawText(ff, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(game.chess[game.chessPoint[i].x()][game.chessPoint[i].y()].seq + 1));
			}
			painter.setPen(Qt::NoPen);
		}
	}
	pen.setColor(Qt::red);
	pen.setWidth(1);
	painter.setPen(pen);
	if ((moveX * 40 + 40) >= 20 && (moveX * 40 + 40) <= 620 && (moveY * 40 + 20) >= 20 && (moveY * 40 + 20) <= 640)
	{
		painter.drawLine((moveY + 1) * 40 - 20, (moveX + 1) * 40, (moveY + 1) * 40 - 10, (moveX + 1) * 40);
		painter.drawLine((moveY + 1) * 40 + 20, (moveX + 1) * 40, (moveY + 1) * 40 + 10, (moveX + 1) * 40);
		painter.drawLine((moveY + 1) * 40 - 20, (moveX + 1) * 40 + 40, (moveY + 1) * 40 - 10, (moveX + 1) * 40 + 40);
		painter.drawLine((moveY + 1) * 40 + 20, (moveX + 1) * 40 + 40, (moveY + 1) * 40 + 10, (moveX + 1) * 40 + 40);
		painter.drawLine((moveY + 1) * 40 - 20, (moveX + 1) * 40, (moveY + 1) * 40 - 20, (moveX + 1) * 40 + 10);
		painter.drawLine((moveY + 1) * 40 + 20, (moveX + 1) * 40, (moveY + 1) * 40 + 20, (moveX + 1) * 40 + 10);
		painter.drawLine((moveY + 1) * 40 - 20, (moveX + 1) * 40 + 40, (moveY + 1) * 40 - 20, (moveX + 1) * 40 + 30);
		painter.drawLine((moveY + 1) * 40 + 20, (moveX + 1) * 40 + 40, (moveY + 1) * 40 + 20, (moveX + 1) * 40 + 30);
	}
	if (game.player == 5)
		for (int i = 0; i < game.dadiann&&!game.dafou; i++)
		{
			painter.drawLine((game.chessPoint[4+i].y() + 1) * 40 - 1, (game.chessPoint[4+i].x() + 1) * 40 + 20, (game.chessPoint[4+i].y() + 1) * 40 - 6, (game.chessPoint[4+i].x() + 1) * 40 + 20);
			painter.drawLine((game.chessPoint[4+i].y() + 1) * 40 + 1, (game.chessPoint[4+i].x() + 1) * 40 + 20, (game.chessPoint[4+i].y() + 1) * 40 + 6, (game.chessPoint[4+i].x() + 1) * 40 + 20);
			painter.drawLine((game.chessPoint[4+i].y() + 1) * 40, (game.chessPoint[4+i].x() + 1) * 40 + 19, (game.chessPoint[4+i].y() + 1) * 40, (game.chessPoint[4+i].x() + 1) * 40 + 14);
			painter.drawLine((game.chessPoint[4+i].y() + 1) * 40, (game.chessPoint[4+i].x() + 1) * 40 + 21, (game.chessPoint[4+i].y() + 1) * 40, (game.chessPoint[4+i].x() + 1) * 40 + 26);
		}
	else if (game.player>=1)
	{
		painter.drawLine((game.chessPoint.last().y() + 1) * 40 - 1, (game.chessPoint.last().x() + 1) * 40 + 20, (game.chessPoint.last().y() + 1) * 40 - 6, (game.chessPoint.last().x() + 1) * 40 + 20);
		painter.drawLine((game.chessPoint.last().y() + 1) * 40 + 1, (game.chessPoint.last().x() + 1) * 40 + 20, (game.chessPoint.last().y() + 1) * 40 + 6, (game.chessPoint.last().x() + 1) * 40 + 20);
		painter.drawLine((game.chessPoint.last().y() + 1) * 40, (game.chessPoint.last().x() + 1) * 40 + 19, (game.chessPoint.last().y() + 1) * 40, (game.chessPoint.last().x() + 1) * 40 + 14);
		painter.drawLine((game.chessPoint.last().y() + 1) * 40, (game.chessPoint.last().x() + 1) * 40 + 21, (game.chessPoint.last().y() + 1) * 40, (game.chessPoint.last().x() + 1) * 40 + 26);
	}
}
void ChessPlay::mouseMoveEvent(QMouseEvent *event)
{
	moveX = (event->y() - 40) / 40;
	moveY = (event->x() - 20) / 40;
	update();
}

ChessPlay::~ChessPlay()
{
	delete ui;
}

void ChessPlay::on_actionBack_triggered()
{
	game.AIkongzhi = true;
	while (game.waitThread);
	game.AIkongzhi = false;
	game.backChess();
	game.backChess();
}

void ChessPlay::on_actionQuit_2_triggered()
{
	this->close();
}

void ChessPlay::on_actionHome_triggered()
{
	this->close();
	MainWindow* MW = new MainWindow;
	MW->show();
}

void ChessPlay::on_actionScreenshot_triggered()
{
	QDateTime datetime;
	QString timestr = datetime.currentDateTime().toString("yyyy.MM.dd hh"), firstname, secondname, winer, fileName;
	QString time = datetime.currentDateTime().toString("mm");
	if (game.cPeople = BLACK_CHESS)
	{
		firstname = adversaryName + QStringLiteral(" B");
		secondname = QStringLiteral("八倍镜界 W");
	}
	else
	{
		firstname = QStringLiteral("八倍镜界 B");
		secondname = adversaryName + QStringLiteral(" W");
	}
	if ((game.player - 1) % 2)
		winer = QStringLiteral("后手胜");
	else
		winer = QStringLiteral("先手胜");
	fileName = "C5-" + firstname + " vs " + secondname + "-" + winer + "-" + timestr + QStringLiteral(" ：") + time + QStringLiteral(" 合肥-CCGC");
	fileName = "D:/" + fileName + ".txt";
	QFile file(fileName);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream in(&file);
	in << "{[C5][" + firstname + "][" + secondname + "][" + winer + "][" + timestr + ":" + time + QStringLiteral(" 合肥][2018 CCGC];");
	int m = 0;
	while (m < game.player)
	{
		for (int i = 0; i < 15; i++)
			for (int j = 0; j < 15; j++)
			{
				if (game.chess[i][j].seq == m)
				{
					if ((m + 2) % 2 == 0)
						in << "B(" + QString('A' + j) + "," + QString::number(15 - i) + ");";
					else
						in << "W(" + QString('A' + j) + "," + QString::number(15 - i) + ");";
					m++;
				}
			}
	}
	in << "}";
	file.flush();
	file.close();
	QPixmap screenshot = this->grab(QRect(20, 40, 600, 600));
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("保存图片"), "", "Image File(*.jpg)");
	if (filename.length()>0)
		screenshot.save(filename);
}

void ChessPlay::on_actionGame_triggered()
{
	int ret;
	this->close();
	ChessPlay* pvc = new ChessPlay;
	pvc->show();
	pvc->putinAdversaryName();
	pvc->judgeWhoFirst();
}

void ChessPlay::on_actionPASS_triggered()
{
	game.message.type = 33;
	update();
	timeUpdate();
	mouseflag = false;
	game.AIkongzhi = true;
	while (game.waitThread);
	game.AIkongzhi = false;
	computerPlaychess();
}

void ChessPlay::timeUpdate()
{
	if ((game.player - 1) % 2)
	{
		wtimerId->stop();
		btimerId->start(1000);
	}
	else
	{
		btimerId->stop();
		wtimerId->start(1000);
	}
	update();
}

void ChessPlay::judgeWin()
{
	if (game.ifWin(game.chessPoint.last().x(), game.chessPoint.last().y()) && game.player != 5)
	{
		if (game.chess[game.chessPoint.last().x()][game.chessPoint.last().y()].flag == 5)
			QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("白棋获胜"));
		else
			QMessageBox::about(this, QStringLiteral("恭喜"), QStringLiteral("黑棋获胜"));
		btimerId->stop();
		wtimerId->stop();
		mouseflag = false;
	}
}

void ChessPlay::judgeKinjite(int x, int y)
{
	if (game.judge_IsKinjite(x, y) && game.player != 5)
	{
		QMessageBox box(QMessageBox::Question, QStringLiteral("禁手检测"), QStringLiteral("黑方下了禁手点"));
		box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		box.setButtonText(QMessageBox::Yes, QString(QStringLiteral("结束游戏")));
		box.setButtonText(QMessageBox::No, QString(QStringLiteral("继续游戏")));
		if (box.exec() == QMessageBox::Yes)
		{
			btimerId->stop();
			wtimerId->stop();
			mouseflag = false;
		}
	}
}

void ChessPlay::computerPlaychess()
{
	game.huiqijudge = false;
	if (game.player == 5 && !game.dafou)
	{
		game.daN();
		Nda(game.comX, game.comY);
	}
	game.UpdateLimit();
	if (game.cComputer == BLACK_CHESS)
		game.times = &whiteTimes;
	else
		game.times = &blackTimes;
	gameThread->start();
}

bool ChessPlay::peoplePlaychess(int x, int y)
{
	game.UpdateLimit();
	if (game.player == 4)
		game.dadianN--;
	QPoint point = game.peoplePutdown(x, y);
	game.currentX = x;
	game.currentY = y;
	timeUpdate();
	if (game.player == 4 && game.cComputer == WHITE_CHESS)
	{
		game.chess[x][y].seq = 255;
		return true;
	}
	if (game.player < 3)
		return true;
	if (game.player == 3)
		if (computerExchange())
			return true;
	judgeWin();
	judgeKinjite(x, y);
	mouseflag = false;
	ui->actionPASS->setEnabled(false);
	return false;
}

void ChessPlay::computerPlaychessGoon()
{
	judgeWin();
	game.waitBackAI = true;
	ui->actionPASS->setEnabled(true);
	timeUpdate();
	mouseflag = true;
	game.huiqijudge = true;
}

void ChessPlay::tiaoshizhuanyong(int x, int y)
{
	mouseflag = false;
	QPoint point = game.ceshizhuanyong(x, y);
	judgeKinjite(x, y);
	mouseflag = true;
}

bool ChessPlay::JudgeIsExist(int x, int y)
{
	for (int i = 0; i < game.player; i++)
	{
		if (game.chessPoint[i].x() == x&&game.chessPoint[i].y() == y)
			return false;
	}
	return true;
}
