#ifndef MAINWIN_H
#define MAINWIN_H
#include <QtWidgets>

class mainWin : public QObject{
    Q_OBJECT

    public:
    mainWin(){}
    virtual ~mainWin(){}

    void checkLineInput(QLayout *parent, QLineEdit* line, QWidget *goal, double *value);

public slots:
    void line1Edited();
    void startButtonClicked();
};

#endif // MAINWIN_H
