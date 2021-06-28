#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

class About
{
public:
    static About& getInstance() {
        return instance;
    }
    void about();
private:
    About() {};
    static About instance;
};

namespace Ui
{
  class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUT_H
