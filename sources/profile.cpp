#include <QtCore/QProcess>
#include <QtCore/QRegExp>
#include <QtCore/QVariant>
#include <QtGui/QHBoxLayout>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "profile.hpp"


DirectoryProfile::DirectoryProfile()
{

}

bool DirectoryProfile::run(const void* runctx)
{
    QString excmd = m_programexeccmd;
    // prepare profile path
    if (excmd.indexOf("$path") != -1)
        excmd.replace("$path", m_profiledir);

    return QProcess::startDetached(excmd);
}


CryptMountProfile::CryptMountProfile():
    DirectoryProfile(),
    m_lasterror(""),
    m_mountpassword(""),
    m_listwidget(0)
{

}

CryptMountProfile::~CryptMountProfile()
{

}

int CryptMountProfile::cryptmount_mount()
{
    int ret;
    QStringList args;
    args << target() << "-w" << "0";

    QProcess proc;
    proc.start("cryptmount", args, QIODevice::ReadWrite);
    proc.waitForStarted();

    // write password
    proc.write(QString(m_mountpassword + '\n').toStdString().c_str());
    proc.closeWriteChannel();
    proc.waitForBytesWritten();
    proc.waitForFinished();

    int icode  = proc.exitCode();
    int retcode = 0;

    // see if 0 (mount success) or 29 (target already mounted)
    if ((icode == 0) || (icode == 29))
        // mount successful / already mounted
        retcode = 0;
    else if (icode == 21)
        // wrong password
        retcode = -1;
    else
    {
        // something else
        m_lasterror = proc.errorString();
        qCritical("Failed to do cryptmount, returned message was:\n%s", m_lasterror.toStdString().c_str());
        qCritical("Return code: %d", icode);
        retcode = -2;
    }

    // reset password
    m_mountpassword = "";
    // update button caption
    updateButtonToggle();
    // return value to caller
    return retcode;
}


QWidget* CryptMountProfile::getListWidget(QWidget* list)
{
    if (!m_listwidget)
    {
        QWidget* wdg = new QWidget(list);
        wdg->setProperty("profile", qVariantFromValue< void* >(this));

        QPushButton* btn = new QPushButton(wdg);
        btn->setObjectName("mountPushButton");
        updateButtonToggle(btn);

        QHBoxLayout* lyt = new QHBoxLayout(wdg);
        lyt->addStretch();
        lyt->addWidget(btn);
        lyt->setMargin(0);

        m_listwidget = wdg;
    }

    return m_listwidget;
}

void CryptMountProfile::inspectTarget()
{
    QStringList args;
    args << "-l";

    QRegExp targetline("([\\w-]+)\\s*\\[to mount on \"(.*)\" as \"(.*)\"\\]");

    QProcess proc;
    proc.start("cryptmount", args, QIODevice::ReadOnly);
    proc.waitForStarted();
    proc.waitForReadyRead();

    while (proc.canReadLine())
    {
        QString linebuf = QString(proc.readLine()).trimmed();
        if (linebuf.length() < 1)
            continue;

        if (targetline.exactMatch(linebuf) && (targetline.cap(1) == target()))
        {
            m_filesystem = targetline.cap(3);
            setDirectory( targetline.cap(2) );
            break;
        }
    }
    proc.close();
}

bool CryptMountProfile::isMounted()
{
    bool ret = false;
    QString mappedpath = "/dev/mapper/" + target();
    QRegExp mountline("^(.+) on (.+) type (.+) \\((.+)\\)$");

    QProcess proc;
    proc.start("mount", QIODevice::ReadOnly);
    proc.waitForStarted();
    proc.waitForReadyRead();

    while (proc.canReadLine())
    {
        QString linebuf = QString(proc.readLine()).trimmed();
        if (linebuf.length() < 1)
            continue;

        if (mountline.exactMatch(linebuf))
            if (  (mountline.cap(1) == mappedpath) &&
                  (mountline.cap(2) == directory()) &&
                  (mountline.cap(3) == m_filesystem))
            {
                ret = true;
                break;
            }
    }

    proc.close();
    return ret;
}

int CryptMountProfile::mount(const void* mntctx)
{
    CryptMountProfileMountContext* mntctx_ = (CryptMountProfileMountContext*)mntctx;

    while (mntctx_)
    {
        QWidget* parent = mntctx_->parentWidget;

        QString pwstr;
        bool pwok = false;
        pwstr = QInputDialog::getText(parent, parent->trUtf8("Password"),
                                      parent->trUtf8("Enter mount password"),
                                      QLineEdit::Password, pwstr, &pwok);
        if (pwok)
            setPassword(pwstr);
        else
            break;

        int mountcode = cryptmount_mount();
        if (mountcode == 0)
            return mountcode;
        else if (mountcode == -1)
        {
            // inform user, password is invalid, stay in the loop
            QMessageBox::information(parent, parent->trUtf8("Password"),
                                     parent->trUtf8("Wrong password entered."));
        }
        else
        {
            // dump error
            QMessageBox::warning(parent, parent->trUtf8("Mount Error"), lastError());
            return mountcode;
        }
    }
}

bool CryptMountProfile::run(const void* runctx)
{
    bool prepok = true;
    CryptMountProfileRunContext* runctx_ = (CryptMountProfileRunContext*)runctx;
    if (runctx_ && runctx_->mountFirst && !isMounted())
    {
        CryptMountProfileMountContext mntctx;
        mntctx.parentWidget = runctx_->parentWidget;

        prepok = mount(&mntctx) == 0;
    }

    if (prepok)
        return DirectoryProfile::run(0);

    return false;
}

void CryptMountProfile::setTarget(const QString& tgtname)
{
    if (m_targetname != tgtname)
    {
        m_targetname = tgtname;
        inspectTarget();
    }
}

int CryptMountProfile::unmount(const void* umntctx)
{
    int ret;
    QStringList args;
    args << "-u" << target();

    QProcess proc;
    proc.start("cryptmount", args, QIODevice::ReadOnly);
    proc.waitForStarted();
    proc.waitForFinished();

    int retcode  = proc.exitCode();

    // update button caption
    updateButtonToggle();
    // return value to caller
    return retcode;
}

void CryptMountProfile::updateButtonToggle(QPushButton* btn)
{
    if (!btn && m_listwidget)
        btn = m_listwidget->findChild< QPushButton* >("mountPushButton");

    if (!isMounted())
        btn->setText(QObject::trUtf8("Mount"));
    else
        btn->setText(QObject::trUtf8("Unmount"));
}
