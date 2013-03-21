#ifndef profile_hpp
#define profile_hpp

#include <QtCore/QString>
#include <QtGui/QWidget>


enum ProfileType
{
    PT_UNKNOWN    = 0,
    PT_DIRECTORY  = 1,
    PT_CRYPTMOUNT = 2
};


struct CryptMountProfileMountContext
{
    QWidget* parentWidget;
};

struct CryptMountProfileRunContext
{
    QWidget* parentWidget;
    bool     mountFirst;
};


class Profile
{
protected:
    QString      m_name;
    QString      m_profiledir;
    QString      m_programexeccmd;

public:
    virtual ~Profile() {};

    virtual ProfileType  profileType() { return PT_UNKNOWN; }
    virtual QString  directory() const { return m_profiledir; }
    virtual QString  name() const { return m_name; }
    virtual QWidget*  getListWidget(QWidget* parent) = 0;
    virtual bool run(const void* runctx=0) = 0;
    virtual int  mount(const void* mntctx=0) = 0;
    virtual int  unmount(const void* umntctx=0) = 0;
    virtual void  setDirectory(const QString& dir) { m_profiledir = dir; }
    virtual void  setExecCommand(const QString& execcmd) { m_programexeccmd = execcmd; }
    virtual void  setName(const QString& pname) { m_name = pname; }
};


class DirectoryProfile:
    public Profile
{
public:
    DirectoryProfile();
    virtual ~DirectoryProfile() {}

    virtual ProfileType  profileType() { return PT_DIRECTORY; }
    virtual QWidget*  getListWidget(QWidget* parent) { return 0; }
    virtual bool  run(const void* runctx);
    virtual int  mount(const void* mntctx=0) {};
    virtual int  unmount(const void* umntctx=0) {};
};


class CryptMountProfile:
    public DirectoryProfile
{
protected:
    QString   m_filesystem;
    QString   m_lasterror;
    QString   m_mountpassword;
    QString   m_targetname;
    QWidget*  m_listwidget;

    void inspectTarget();
    void updateButtonToggle(QPushButton* btn=0);
    int cryptmount_mount();

public:
    CryptMountProfile();
    virtual ~CryptMountProfile();

    QString  lastError() const { return m_lasterror; }
    QString  target() const { return m_targetname; }
    bool  isMounted();
    virtual ProfileType  profileType() { return PT_CRYPTMOUNT; }
    virtual QWidget*  getListWidget(QWidget* list);
    virtual bool  run(const void* runctx);
    virtual int  mount(const void* mntctx=0);
    virtual int  unmount(const void* umntctx=0);
    void setPassword(const QString& pw) { m_mountpassword = pw; }
    void setTarget(const QString& tgtname);
};

#endif // profile_hpp
