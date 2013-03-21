#ifndef selectprofilesdlg_hpp
#define selectprofilesdlg_hpp

#include <QtGui/QDialog>

#include "profile.hpp"
#include "ui_selectprofilesdlg.h"


class CSelectProfilesDlg:
    public QDialog,
    private Ui::SelectProfilesDlg
{
Q_OBJECT

private:
    QList< Profile* >  m_profiles;

    bool readFile(const QString& fname);
    void clearProfiles();
    void loadProfiles();
    bool launchProfile(Profile* prof);

private slots:
    void on_cryptmountitem_mounttoggled(bool down);
    void on_launchclicked(bool down);
    void on_launchprofile(const QModelIndex& index);

public:
    CSelectProfilesDlg(QWidget* parent=0, Qt::WindowFlags flags=0);
    virtual ~CSelectProfilesDlg();

    virtual void setupUi(QDialog* dlg);
    public slots:
    public slots:
};

#endif // selectprofilesdlg_hpp
