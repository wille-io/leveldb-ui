#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QString preDirectory;
  if (a.arguments().count() > 1)
    preDirectory = a.arguments().at(1);

  MainWindow w(preDirectory);
  w.show();

  return a.exec();
}
