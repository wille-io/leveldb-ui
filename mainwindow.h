#pragma once

#include <QMainWindow>


namespace leveldb
{
class DB;
}

namespace Ui
{
class MainWindow;
}


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(const QString &preDirectory, QWidget *parent = 0);
  ~MainWindow();

private slots:
  void slotOpenDatabase();
  void slotDirectorySelected(const QString &dir);
  void slotKeySelected(const QModelIndex &index);
  void slotOpen();
  void slotAbout();
  void slotCountAllKeys();
  void slotDeleteSelectedEntries();

private:
  Ui::MainWindow *ui;
  leveldb::DB *mDb;
  QString mPreDirectory;

};
