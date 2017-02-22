/* qTorrent - An open-source, cross-platform BitTorrent client
 * Copyright (C) 2017 Petko Georgiev
 *
 * mainwindow.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qtorrent.h"
#include "mainwindow.h"
#include "torrentslist.h"
#include "panel.h"
#include "torrentinfopanel.h"
#include "addtorrentdialog.h"
#include <QGuiApplication>
#include <QScreen>
#include <QStackedWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCloseEvent>
#include <QApplication>

const int UI_REFRESH_INTERVAL = 300;

MainWindow::MainWindow()
	: m_panel(new Panel)
	, m_torrentsList(new TorrentsList())
	, m_infoPanel(new TorrentInfoPanel)
{
	// Set the main window size to 3/4 of the screen size
	int width = QGuiApplication::primaryScreen()->size().width()*3/4;
	int height = QGuiApplication::primaryScreen()->size().height()*3/4;
	resize(width, height);

	addToolBar(Qt::LeftToolBarArea, m_panel);
	addToolBar(Qt::BottomToolBarArea, m_infoPanel);
	QStackedWidget* stackedWidget = new QStackedWidget;
	stackedWidget->addWidget(m_torrentsList);
	setCentralWidget(stackedWidget);

	connect(m_panel, SIGNAL(showAll()), m_torrentsList, SLOT(showAll()));
	connect(m_panel, SIGNAL(showCompleted()), m_torrentsList, SLOT(showCompleted()));
	connect(m_panel, SIGNAL(showDownloading()), m_torrentsList, SLOT(showDownloading()));
	connect(m_panel, SIGNAL(showUploading()), m_torrentsList, SLOT(showUploading()));

	m_panel->openAll();

	createMenus();

	m_refreshTimer.setInterval(UI_REFRESH_INTERVAL);
	m_refreshTimer.start();
	connect(&m_refreshTimer, SIGNAL(timeout()), m_torrentsList, SLOT(refresh()));
	connect(&m_refreshTimer, SIGNAL(timeout()), m_infoPanel, SLOT(refresh()));

	m_trayIconMenu = new QMenu(this);
	m_trayIconMenu->addAction(tr("Hide/Show qTorrent"), this, &MainWindow::toggleHideShow);
	m_trayIconMenu->addAction(tr("Exit"), this, &MainWindow::exitAction);

	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setContextMenu(m_trayIconMenu);

	m_trayIcon->show();

	connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

MainWindow::~MainWindow() {

}

Panel* MainWindow::panel() {
	return m_panel;
}

TorrentsList* MainWindow::torrentsList() {
	return m_torrentsList;
}


void MainWindow::addTorrent(Torrent *torrent) {
	m_torrentsList->addTorrent(torrent);
}

void MainWindow::removeTorrent(Torrent *torrent) {
	m_torrentsList->removeTorrent(torrent);
}


void MainWindow::createMenus() {
	QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("&Add torrent"), this, &MainWindow::addTorrentAction);
	fileMenu->addAction(tr("&Exit"), this, &MainWindow::exitAction);
}


QString MainWindow::getDownloadLocation() {
	// Open a dialog box to select the download directory
	QString downloadPath;
	downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
	downloadPath = QFileDialog::getExistingDirectory(this, tr("Select download directory"), downloadPath);
	// String is empty if user canceled the dialog box
	return downloadPath;
}

void MainWindow::addTorrentAction() {
	AddTorrentDialog dialog(this);
	dialog.exec();
}

void MainWindow::exitAction() {
	if(QTorrent::instance()->question("Are you sure you want to exit " + QGuiApplication::applicationDisplayName() + "?")) {
		QApplication::quit();
	}
}

void MainWindow::addTorrentFromUrl(QUrl url) {
	AddTorrentDialog dialog(this);
	dialog.setTorrentUrl(url);
	dialog.exec();
}

void MainWindow::closeEvent(QCloseEvent *event) {
	event->ignore();
	hide();
}

void MainWindow::toggleHideShow() {
	if(isHidden()) {
		show();
	} else {
		hide();
	}
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	if(reason == QSystemTrayIcon::DoubleClick) {
		show();
	}
}
