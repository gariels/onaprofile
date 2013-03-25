#include <iostream>
#include <unistd.h>
#include <stdint.h>

#include "selectprofilesdlg.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include <qjson/parser.h>


CSelectProfilesDlg::CSelectProfilesDlg(QWidget* parent, Qt::WindowFlags flags):
    QDialog(parent, flags)
{
    setupUi(this);
    loadProfiles();
}

CSelectProfilesDlg::~CSelectProfilesDlg()
{
    clearProfiles();
}

void CSelectProfilesDlg::clearProfiles()
{
    QListWidget* listw = this->findChild< QListWidget* >("profilesListWidget");
    for (int idx = 0; idx < listw->count(); ++idx)
    {
        QListWidgetItem* lwi = listw->item(idx);
        void* ptr = lwi->data(Qt::UserRole).value< void* >();
        Profile* prof = static_cast< Profile* >(ptr);
        switch (prof->profileType())
        {
            case PT_DIRECTORY:
                delete static_cast< DirectoryProfile* >(prof);
                break;

            case PT_CRYPTMOUNT:
                delete static_cast< CryptMountProfile* >(prof);
                break;

            default:
                delete prof;
        }
    }
    m_profiles.clear();
}

bool CSelectProfilesDlg::launchProfile(Profile* prof)
{
    if (!prof)
        return false;

    void* ctxptr = 0;
    if (prof->profileType() == PT_CRYPTMOUNT)
    {
        CryptMountProfileRunContext runctx;
        runctx.mountFirst = true;
        runctx.parentWidget = this;

        ctxptr = &runctx;
    }

    std::cout << "Running profile \"" << prof->name().toStdString() << "\" ..." << std::endl;
    return prof->run(ctxptr);
}

void CSelectProfilesDlg::loadProfiles()
{
    QCoreApplication* app = QCoreApplication::instance();
    QString configfn;

    int configidx = app->arguments().indexOf("--config");
    if ((configidx != -1) && (configidx < app->arguments().count()))
        configfn = app->arguments()[configidx + 1];

    if (configfn.isEmpty())
    {
        qCritical("Unable to load configuration file. Please specify it by using \"--config\" parameter.");
        exit(1);
    }
    else if (!readFile(configfn))
        exit(1);

    QListWidget* listw = this->findChild< QListWidget* >("profilesListWidget");
    if (listw->count() > 0)
        listw->setCurrentRow(0);
}

void CSelectProfilesDlg::on_cryptmountitem_mounttoggled(bool down)
{
    QWidget* wdg = static_cast< QWidget* >(sender());
    if (!wdg)
        return;

    QVariant wprof = wdg->parent()->property("profile");
    if (!wprof.isValid())
        return;

    CryptMountProfile* prof = static_cast< CryptMountProfile* >(wprof.value< void* >());
    if (!prof)
        return;

    if ( prof->isMounted() )
        prof->unmount();
    else
    {
        CryptMountProfileMountContext mntctx;
        mntctx.parentWidget = this;

        prof->mount(&mntctx);
    }
}

void CSelectProfilesDlg::on_launchclicked(bool down)
{
    QListWidget* listw = this->findChild< QListWidget* >("profilesListWidget");

    QModelIndex index = listw->currentIndex();
    if (!index.isValid())
        return;

    void* ptr = index.data(Qt::UserRole).value< void* >();
    if (!ptr)
        return;

    Profile* profile = static_cast< Profile* >(ptr);
    if (launchProfile(profile))
        close();
}

void CSelectProfilesDlg::on_launchprofile(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    void* ptr = index.data(Qt::UserRole).value< void* >();
    if (!ptr)
        return;

    Profile* profile = static_cast< Profile* >(ptr);
    launchProfile(profile);
}

bool CSelectProfilesDlg::readFile(const QString& fname)
{
    QFile rsfile(fname);
    if (!rsfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical("Unable to open %s for read.", fname.toStdString().c_str());
        return false;
    }

    bool parseok;
    QJson::Parser parser;
    QVariantMap json = parser.parse(&rsfile, &parseok).toMap();
    if (!parseok)
    {
        qCritical("Unable to parse %s", fname.toStdString().c_str());
        return false;
    }

    if (!(json.contains("title") && json.contains("exec") && json.contains("profiles")))
    {
        qCritical("%s parsed as invalid configuration file.", fname.toStdString().c_str());
        return false;
    }

    QListWidget* listw = this->findChild< QListWidget* >("profilesListWidget");
    QVariantMap profiles = json["profiles"].toMap();
    foreach (QVariant prof, profiles)
    {
        QVariantMap profmap = prof.toMap();
        if (!profmap.contains("type"))
            continue;

        QString ptype = profmap["type"].toString();
        if (ptype.compare("dir", Qt::CaseInsensitive) == 0 ||
            ptype.compare("directory", Qt::CaseInsensitive) == 0)
        {
            DirectoryProfile* profile = new DirectoryProfile();
            profile->setName(profiles.key(prof));
            profile->setExecCommand(json["exec"].toString());

            if (profmap.contains("path") && !profmap["path"].toString().isEmpty())
                profile->setDirectory(profmap["path"].toString());

            QVariant pdata = qVariantFromValue< void* >(profile);
            QListWidgetItem* lwi = new QListWidgetItem(listw);
            lwi->setData(Qt::UserRole, pdata);
            lwi->setText(profile->name());
            lwi->setSizeHint(QSize(0, 24));
        }
        else if (ptype.compare("cryptmount", Qt::CaseInsensitive) == 0)
        {
            CryptMountProfile* profile = new CryptMountProfile();
            profile->setName(profiles.key(prof));
            profile->setExecCommand(json["exec"].toString());

            QVariantMap typedata = profmap["typedata"].toMap();
            if (typedata.contains("target") && !typedata["target"].toString().isEmpty())
                profile->setTarget(typedata["target"].toString());

            QVariant pdata = qVariantFromValue< void* >(profile);
            QListWidgetItem* lwi = new QListWidgetItem(listw);
            lwi->setData(Qt::UserRole, pdata);
            lwi->setText(profile->name());
            lwi->setSizeHint(QSize(0, 24));

            // create mount/unmount widget
            QWidget* wdg = profile->getListWidget(listw);
            QPushButton* btn = wdg->findChild< QPushButton* >("mountPushButton");
            if (btn)
                connect(btn, SIGNAL( clicked(bool) ), this,
                        SLOT(on_cryptmountitem_mounttoggled(bool)));

            listw->setItemWidget(lwi, wdg);
        }
    }

    return true;
}

void CSelectProfilesDlg::setupUi(QDialog* dlg)
{
    Ui::SelectProfilesDlg::setupUi(dlg);

    setWindowTitle(trUtf8("On A Profile"));
    setWindowIcon(QIcon("://onaprofile.png"));

    QListWidget* listw = this->findChild< QListWidget* >("profilesListWidget");
    listw->setUniformItemSizes(false);
    listw->setSelectionBehavior(QAbstractItemView::SelectRows);
    listw->setSpacing(2);
    listw->setIconSize(QSize(24, 24));
    listw->setFocus();

    connect(listw, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_launchprofile(QModelIndex)));

    QDialogButtonBox* dialogbtnbox = this->findChild< QDialogButtonBox* >("buttonBox");
    if (dialogbtnbox)
    {
        QPushButton* btn = dialogbtnbox->button(QDialogButtonBox::Ok);
        if (btn)
        {
            btn->setText(trUtf8("Launch"));
            btn->setIcon(QIcon("://launch.png"));
            connect(btn, SIGNAL(clicked(bool)), this, SLOT(on_launchclicked(bool)));
        }
    }

}
