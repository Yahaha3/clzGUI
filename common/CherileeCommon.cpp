#include<qsystemdetection.h>
#ifdef Q_OS_WIN32
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#elif defined(Q_OS_LINUX)
    ///TODO:Linux
#endif

#include "CherileeCommon.h"
#include <QJsonDocument>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <iostream>
#include <QDataStream>
#include <QFileDialog>
#include <QSettings>

std::string WStringToString(const std::wstring &wstr)
{
    std::string str(wstr.length(), ' ');
    std::copy(wstr.begin(), wstr.end(), str.begin());
    return str;
}

clz::CherileeCommon::CherileeCommon()
{

}

QJsonObject clz::CherileeCommon::read_json_object_file(const QString &path)
{
    QFile file(path);
    if (!QFileInfo(path).exists()) {
        QString err =
                QObject::tr("No such json file ")
                + "("+ path + ")";
        qDebug() << err;
    }
    if (!file.open(QIODevice::ReadOnly)) return QJsonObject();

    QByteArray qba = file.readAll();

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(qba, &error);
    if (error.error != QJsonParseError::NoError)
    {
        int char_offset = error.offset;
        QByteArray qba_left = qba.left(char_offset);
        int row = qba_left.count("\n");
        int offset = char_offset - qba_left.lastIndexOf("\n");
        QString err =
                error.errorString()
                + "\n @row: " + QString::number(row) +
                + "\n @offset: " + QString::number(offset);
        qDebug() << err;
        return QJsonObject();
    }

    file.close();

    if (!document.isObject())
    {
        QString s;
        QTextStream ss(&s);
        ss << QObject::tr("The document is invalid json object.");
    }
    return document.object();
}

QJsonArray clz::CherileeCommon::read_json_array_file(const QString &path)
{
    QFile file(path);
    if (!QFileInfo(path).exists()) {
        QString err =
                QObject::tr("No such json file ")
                + "("+ path + ")";
        qDebug() << err;
    }
    if (!file.open(QIODevice::ReadOnly)) return QJsonArray();

    QByteArray qba = file.readAll();

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(qba, &error);
    if (error.error != QJsonParseError::NoError)
    {
        int char_offset = error.offset;
        QByteArray qba_left = qba.left(char_offset);
        int row = qba_left.count("\n");
        int offset = char_offset - qba_left.lastIndexOf("\n");
        QString err =
                error.errorString()
                + "\n @row: " + QString::number(row) +
                + "\n @offset: " + QString::number(offset);
        qDebug() << err;
        return QJsonArray();
    }

    file.close();
    if (!document.isArray())
    {
        QString s;
        QTextStream ss(&s);
        ss << QObject::tr("The document is invalid jsonarray object.");
    }
    return document.array();
}

QJsonObject clz::CherileeCommon::qstring_to_json_object(const QString &string)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());

    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            return obj;
        }
    }
    else
    {
        return obj;
    }
    return obj;
}

QString clz::CherileeCommon::json_object_to_qstring(const QJsonObject &jo)
{
    QJsonDocument doc(jo);
    return QString(doc.toJson(QJsonDocument::Compact));
}

QStringList clz::CherileeCommon::json_array_to_stringlist(const QJsonArray &qja)
{
    QStringList list;
    for(auto qjo: qja){
        auto jo = qjo.toObject();
        list.append(json_object_to_qstring(jo));
    }
    return list;
}

QString clz::CherileeCommon::json_array_to_string(QJsonArray qja, bool compact)
{
    QJsonDocument doc(qja);
    if (compact) {
        return QString(doc.toJson(QJsonDocument::Compact));
    } else {
        return QString(doc.toJson(QJsonDocument::Indented));
    }
}

QJsonArray clz::CherileeCommon::string_to_json_array(QString string)
{
    QJsonArray array;
    QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());

    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isArray())
        {
            array = doc.array();
        }
        else
        {
            return array;
        }
    }
    else
    {
        return array;
    }
    return array;
}

QString clz::CherileeCommon::read_text_file(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return "";
    }
    QTextStream in(&f);
    return in.readAll();
}

void clz::CherileeCommon::write_text_file_object(const QString &path, const QString &text)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << text;
        file.close();
    }
}

void clz::CherileeCommon::append_log_to_file(const QString &path, const QString &log)
{
    QFile file(path);
    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << log;
        file.close();
    }
}

void clz::CherileeCommon::write_csv_file(const QString &path, const QString &text)
{
    auto str = text + "\n";
    append_log_to_file(path, str);
}

void clz::CherileeCommon::json_join(QJsonObject &target, const QJsonObject &jo)
{
    for(auto key: jo.keys()){
        target.insert(key, jo[key]);
    }
}

QString clz::CherileeCommon::get_root_dir()
{
    return QCoreApplication::applicationDirPath();
}

QProcess *clz::CherileeCommon::start_process_detached(QString working_dir, QString name)
{
    QString aos_app_path;
 #if defined (Q_OS_WINDOWS)
     aos_app_path = QString("%1/%2").arg(working_dir).arg(name + ".exe");
 #elif defined (Q_OS_LINUX) || defined(Q_OS_MAC)
     aos_app_path = QString("%1/%2").arg(working_dir).arg(name);
 #endif
     QFileInfo qfi(aos_app_path);
     if (qfi.exists()) {
         QProcess* p = new QProcess();
         p->setProgram(aos_app_path);
         qint64 pid = 0;
         auto d = p->startDetached(&pid);
         if (!d) {
             std::cout << "eqnx-container (Detached) Failed to start." << std::endl;
             return nullptr;
         } else {
             return p;
         }
     } else {
         std::cout << "Failed to locate eqnx-container executable. @" << aos_app_path.toStdString() << std::endl;
         return nullptr;
     }

}

QStringList clz::CherileeCommon::generate_file_dialog_get_paths(const QString &title, const QString &filter, QWidget *parent)
{
    QFileDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setDirectory(".");
    dlg.setNameFilter(filter);
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setViewMode(QFileDialog::Detail);

    // Execute this dialog
    int res = dlg.exec();
    if (!res) return QStringList();

    // Import kml files
    return dlg.selectedFiles();
}

void clz::CherileeCommon::get_graphic_card_infos(QStringList &infos)
{
    int maxDedicatedVideoMemory=0;
    std::string dedicatedVideoCardString="";
#ifdef Q_OS_WIN32
    // 参数定义
    IDXGIFactory * pFactory;
    IDXGIAdapter * pAdapter;
    std::vector <IDXGIAdapter*> vAdapters;            // 显卡

    // 显卡的数量
    int iAdapterNum = 0;

    // 创建一个DXGI工厂
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

    if (FAILED(hr))
    {
        return;
    }

    // 枚举适配器
    while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        ++iAdapterNum;
    }

    // 信息输出
//    qDebug() << "共获取到" << iAdapterNum << "块显卡" << endl;
    for (size_t i = 0; i < vAdapters.size(); i++)
    {
//        qDebug() << "===============显卡" << (i+1) << "信息===============" << endl;
        // 获取信息
        DXGI_ADAPTER_DESC adapterDesc;
        vAdapters[i]->GetDesc(&adapterDesc);
        // 输出显卡信息
        auto description=WStringToString(std::wstring(adapterDesc.Description));
//        qDebug() << "== 设备描述:" << description.data() << endl;
        auto dedicatedVideoMemory = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
        if(dedicatedVideoMemory>maxDedicatedVideoMemory){
            maxDedicatedVideoMemory=dedicatedVideoMemory;
            dedicatedVideoCardString = description;
        }

        infos << description.data();

//        qDebug() << "== 专用视频内存:" << dedicatedVideoMemory << "M" << endl;
//        qDebug() << "== 共享系统内存:" << adapterDesc.SharedSystemMemory / 1024 / 1024 << "M" << endl;
//        qDebug() << "== 系统视频内存:" << adapterDesc.DedicatedSystemMemory / 1024 / 1024 << "M" << endl;
//        qDebug() << "== 设备ID:" << adapterDesc.DeviceId << endl;
//        qDebug() << "== PCI ID修正版本:" << adapterDesc.Revision << endl;
//        qDebug() << "== 子系统PIC ID:" << adapterDesc.SubSysId << endl;
//        qDebug() << "== 厂商编号:" << adapterDesc.VendorId << endl;

        // 输出设备
        IDXGIOutput * pOutput;
        std::vector<IDXGIOutput*> vOutputs;
        // 输出设备数量
        int iOutputNum = 0;
        while (vAdapters[i]->EnumOutputs(iOutputNum, &pOutput) != DXGI_ERROR_NOT_FOUND)
        {
            vOutputs.push_back(pOutput);
            iOutputNum++;
        }

        if(iOutputNum > 0)
        {
//            qDebug() << "== 获取到" << iOutputNum << "个显示设备:" << endl;
        }

        for (size_t n = 0; n < vOutputs.size(); n++)
        {
//            qDebug() << "== 显示设备" << (n+1) << ":" << endl;
            // 获取显示设备信息
            DXGI_OUTPUT_DESC outputDesc;
            vOutputs[n]->GetDesc(&outputDesc);

            // 获取设备支持
            UINT uModeNum = 0;
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
            UINT flags = DXGI_ENUM_MODES_INTERLACED;

            vOutputs[n]->GetDisplayModeList(format, flags, &uModeNum, 0);
            DXGI_MODE_DESC * pModeDescs = new DXGI_MODE_DESC[uModeNum];
            vOutputs[n]->GetDisplayModeList(format, flags, &uModeNum, pModeDescs);

//            qDebug() << "== 显示设备名称:" << WStringToString(std::wstring(outputDesc.DeviceName)).data() << endl;
//            qDebug() << "== 显示设备当前分辨率:" << outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left << "*" << outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top << endl;

            // 所支持的分辨率信息
//            qDebug() << "== 总共支持"<<uModeNum<<"种分辨率" << endl;
            for (UINT m = 0; m < uModeNum; m++)
            {
                //cout << "== 分辨率:" << pModeDescs[m].Width << "*" << pModeDescs[m].Height << "     刷新率" << (pModeDescs[m].RefreshRate.Numerator) / (pModeDescs[m].RefreshRate.Denominator) << endl;
            }
        }
        vOutputs.clear();
//        qDebug() <<"======================================"<<endl;
//        qDebug() << endl;
    }
    vAdapters.clear();
#elif defined(Q_OS_LINUX)
    ///TODO:Linux
#endif

    //获取独立显卡或者集成显卡最大的显存，单位是M
    //显存低于4G（这里使用3000粗略计算）
    if(maxDedicatedVideoMemory < 3000)
    {
//        qDebug() <<"未检测到独立显卡"<<endl;
    }
    else
    {
//        qDebug() <<"检测到独立显卡："<<dedicatedVideoCardString.data()<<endl;
//        qDebug() <<"专用显存大小："<<maxDedicatedVideoMemory<<"M"<<endl;
    }
}

QStringList clz::CherileeCommon::get_hardware_names()
{
    QStringList hardware_names, infos;
    get_graphic_card_infos(infos);
    QString all_infos;
    for(auto info: infos){
        all_infos += info;
    }

    if(all_infos.contains("NVIDIA")){
        hardware_names.append("cuda");
    }
    if(all_infos.contains("AMD")){
        hardware_names.append("d3d11va");
    }
    if(all_infos.contains("Intel") && all_infos.contains("Graphics")){
        hardware_names.append("d3d11va");
    }

    // 读取显卡配置文件
    const char* graphicCardConfigKey = "graphicCardParam";
    QString graphicCardConfigPath = qApp->applicationDirPath() + "/videoconfig/graphicCardConfig.ini";
    QFile file(graphicCardConfigPath);
    if (!file.exists())
    {
        // 不存在则创建
        file.open(QIODevice::WriteOnly);
        file.close();

        QSettings settings(graphicCardConfigPath, QSettings::IniFormat);
        //根据硬解码类型找到对应的硬解码格式
#ifdef HARDWARE_SPEED
        settings.setValue(graphicCardConfigKey, "");
#endif
    }

    QSettings settings(graphicCardConfigPath, QSettings::IniFormat);

#ifdef HARDWARE_SPEED
    QString customHardwareName = settings.value(graphicCardConfigKey).toString();

    if (!customHardwareName.isEmpty())
    {
        hardware_names.clear();
        hardware_names << customHardwareName;

        std::cout << "[VideoPlayer] use custom config: " << customHardwareName.toStdString() << std::endl;
    }
#endif
    return hardware_names;
}
