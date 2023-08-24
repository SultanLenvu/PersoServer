#ifndef GUI_H
#define GUI_H

#include <QApplication>
#include <QDesktopWidget>
#include <QObject>
#include <QtWidgets>

class GUI : public QWidget {
  Q_OBJECT
 public:
  enum GUI_Type { InitialConfiguration, Master, Production };

 protected:
  GUI_Type Type;

 public:
  QHBoxLayout* MainLayout;

  QGroupBox* LogGroup;
  QVBoxLayout* LogLayout;
  QPlainTextEdit* LogDisplay;

 public:
  explicit GUI(QWidget* parent, GUI_Type type);
  virtual ~GUI();

  virtual void create(void) = 0;
  virtual void update(void) = 0;

  GUI_Type type(void);

 public slots:
  void displayLog(const QString& data);
  void clearLogDisplay(void);

 protected:
  void createLog(void);

 signals:
};

#endif  // GUI_H
