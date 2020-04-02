#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTimer>
#include <QLocale>
#include <QDebug>

#include <leveldb/db.h>


MainWindow::MainWindow(const QString &preDirectory, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mDb(nullptr)
  , mPreDirectory(preDirectory)
{
  ui->setupUi(this);

  ui->menuDatabase->setEnabled(false);

  connect(ui->actionOpen, &QAction::triggered,
          this, &MainWindow::slotOpen);
  connect(ui->actionExit, &QAction::triggered,
          this, &MainWindow::close);
  connect(ui->actionAbout_Qt, &QAction::triggered,
          qApp, &QApplication::aboutQt);
  connect(ui->actionAbout_leveldb_ui, &QAction::triggered,
          this, &MainWindow::slotAbout);
  connect(ui->actionCount_all_keys, &QAction::triggered,
          this, &MainWindow::slotCountAllKeys);
  connect(ui->actionDelete_selected_entry, &QAction::triggered,
          this, &MainWindow::slotDeleteSelectedEntries);

  connect(ui->keys, &QListWidget::activated,
          this, &MainWindow::slotKeySelected);

  if (!mPreDirectory.isEmpty())
    QTimer::singleShot(250 /* quick solution for slow decorators, otherwise messages will not be corretly parented by the window manager */,
                       this, &MainWindow::slotOpenDatabase);
}


MainWindow::~MainWindow()
{
  delete mDb;
  delete ui;
}


void MainWindow::slotDeleteSelectedEntries()
{
  const QList<QListWidgetItem *> items(ui->keys->selectedItems());
  qWarning() << "items" << items.count();

  for (QListWidgetItem *item : qAsConst(items))
  {
    QString key(item->data(Qt::ItemDataRole::DisplayRole).toString());

    qWarning() << "key" << key;

    if (mDb->Delete({}, key.toStdString()).ok())
      delete item;
    else
      qWarning() << "could not delete key" << key;
  }
}


void MainWindow::slotCountAllKeys()
{
  uint32_t keyCount = 0;
  leveldb::Iterator* it = mDb->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next())
  {
    keyCount++;
  }

  delete it;

  QMessageBox::information(this, tr("Database key count"), tr("Total keys: %1").arg(QLocale().toString(keyCount)));
}


void MainWindow::slotOpen()
{
  QFileDialog *fileDialog = new QFileDialog(this, tr("Path to database"), mPreDirectory);
  fileDialog->setAttribute(Qt::WA_DeleteOnClose);
  fileDialog->setFileMode(QFileDialog::Directory);
  fileDialog->setOption(QFileDialog::ShowDirsOnly, true);

  connect(fileDialog, &QFileDialog::fileSelected,
          this, &MainWindow::slotDirectorySelected,
          Qt::QueuedConnection);

  fileDialog->show();
}


void MainWindow::slotAbout()
{
  QMessageBox::about(this, tr("leveldb-ui"),
                     tr("<html>"
                        "  <a href='https://github.com/wille-io/leveldb-ui'>github.com/wille-io/leveldb-ui</a><br>"
                        "  <a href='https://wille.io'>wille.io</a> - 2018"
                        "</html>"));
}


void MainWindow::slotKeySelected(const QModelIndex &index)
{
  QString key(index.data().toString());

  std::string value;
  leveldb::Status result = mDb->Get(leveldb::ReadOptions(), key.toStdString(), &value);

  if (!result.ok())
  {
    QMessageBox::critical(this, tr("Cannot read value of key"), QString::fromStdString(result.ToString()));
    return;
  }

  ui->value->setPlainText(QString::fromStdString(value));
  ui->leKey->setText(key);
}


void MainWindow::slotOpenDatabase()
{
  if (mPreDirectory.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot open database"), tr("No directory selected"));
    return;
  }

  leveldb::Options options; // TODO: ask for compression
  leveldb::DB *db;
  leveldb::Status status = leveldb::DB::Open(options, mPreDirectory.toStdString(), &db);

  if (!status.ok())
  {
    QMessageBox::critical(this, tr("Cannot open database"), QString::fromStdString(status.ToString()));
    return;
  }

  // cleanup ui for new db
  ui->keys->clear();
  ui->value->clear();

  if (mDb != nullptr) // close currently open db, if any
    delete mDb;

  mDb = db;


  uint32_t keyCount = 0;
  leveldb::Iterator* it = mDb->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid() && keyCount < 100000; it->Next())
  {
    new QListWidgetItem(QString::fromStdString(it->key().ToString()), ui->keys);
    keyCount++;
  }

  delete it;

  if (keyCount >= 99999)
    QMessageBox::warning(this, tr("Reading database"), tr("There are more than 100.000 entries, stopped there."));

  ui->lbKeyCount->setText(QString::number(keyCount));

  ui->menuDatabase->setEnabled(true);


  {
    std::string val;
    qDebug() << "has prop?" << mDb->GetProperty("leveldb.stats", &val);
    qDebug() << "leveldb.stats" << QString::fromStdString(val);
  }


  {
    std::string val;
    qDebug() << "has prop leveldb.approximate-memory-usage?" << mDb->GetProperty("leveldb.approximate-memory-usage", &val);
    qDebug() << "leveldb.approximate-memory-usage" << QString::fromStdString(val);
  }

}


void MainWindow::slotDirectorySelected(const QString &dir)
{
  mPreDirectory = dir;
  slotOpenDatabase();
}
