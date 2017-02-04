#include "udpserver.h"

UDPServer::UDPServer(QWidget *parent) : QDialog(parent), old_size(0)
{
    Layout();

    server = new QUdpSocket(this);
    if(server->bind(2323)) text->append("[Server]: Server started...");
    else
    {
        connect(this, SIGNAL(Close()), qApp, SLOT(quit()));
        QMessageBox::critical(this, "Error", "Unable server", QMessageBox::Ok);
        emit Close();
    }

    connect(server, SIGNAL(readyRead()), this, SLOT(ProcessData()));

}

UDPServer::~UDPServer()
{

}

void UDPServer::Layout()
{
    QPalette pal;
    pal.setColor(this->backgroundRole(), QColor(90, 90, 90));

    text = edit();
    line = new QLineEdit("Send system message");
    list = new QListWidget;
    list->setStyleSheet("background-color: rgb(137, 197, 212)");

    QPushButton *send = button("Send");
    connect(send, SIGNAL(clicked(bool)), this, SLOT(SendDatagram()));

    download = new QCheckBox("Download users file");
    download->setCheckable(true);
    download->setChecked(true);

    QGridLayout *lay = new QGridLayout;
    lay->addWidget(new QLabel("<b>System messages:</b>"), 0, 0);

    lay->addWidget(text, 1, 0);
    lay->addWidget(line, 2, 0);
    lay->addWidget(send, 3, 0);

    lay->addWidget(new QLabel("<b>Connections:</b>"), 0, 1);
    lay->addWidget(list, 1, 1);
    lay->addWidget(download, 2, 1);

    this->setWindowTitle("Blur/Server");
    this->setLayout(lay);
    this->setPalette(pal);
    this->setWindowIcon(QPixmap("icon.png"));
    this->setFixedSize(550, 400);
}

QTextEdit *UDPServer::edit()
{
    QTextEdit *txt = new QTextEdit;
    txt->setReadOnly(true);
    txt->setFrameStyle(2);

    txt->setStyleSheet("color: white; background-color: rgb(37, 37, 37)");
    txt->setFont(QFont("Ubuntu", 8, QFont::Bold));


    return txt;
}

QPushButton *UDPServer::button(QString name)
{
    QPushButton *btn = new QPushButton(name);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFont(QFont("Ubuntu", 10, QFont::Bold));

    return btn;
}

void UDPServer::AddClient(QString &nickname)
{
    list->setIconSize(QSize(30, 30));
    list->setFont(QFont("Ubuntu", 10, QFont::Bold));
    QListWidgetItem *item = new QListWidgetItem(nickname, list);
    item->setIcon(QPixmap("online.gif"));
    items.push_back(item);
}

void UDPServer::DelClient(QString &nickname)
{
    for(int i=0; i<items.size(); i++)
    {
        if(items[i]->text() == nickname)
        {
            delete items[i];
            items.remove(i);
            break;
        }
    }
}

void UDPServer::SendClientList(QString message)
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    QString name = QString::number(items.size());
    QString str = message;
    out << name << message;

    for(int i=0; i<items.size(); i++)
        out << items[i]->text();

    server->writeDatagram(datagram, QHostAddress::LocalHost, 2424);
}

void UDPServer::Monitor()
{
    if(old_size != items.size())
    {
        old_size = items.size();
        SendClientList("%List%");
    }
}

void UDPServer::SendDatagram()
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    text->append("[Server]: " + line->text());
    QString name = "[Server]";
    out << name << line->text();

    QMap<QString, QHostAddress>::iterator it = addreses.begin();
    for(int i=0; i<addreses.size(); i++)
    {
        server->writeDatagram(datagram, it.value(), 2424); // Sending message to all client
        it++;
    }

    line->clear();
    Monitor();
}

void UDPServer::ProcessData()
{
    QByteArray datagram;
    QHostAddress adr;

    do
        {
            datagram.resize(server->pendingDatagramSize());
            server->readDatagram(datagram.data(), datagram.size(), &adr);

        } while(server->hasPendingDatagrams()); // getting data from client

    QString message, name;
    QDataStream in(&datagram, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_7);
    in >> name;
    in >> message;

    if(message == "%New connection%")
    {
        text->append(message + " - [" + name + "]");
        addreses[name] = adr;

        AddClient(name);
    }

    else if(message == "%Disconnect%")
    {
        text->append(message + " - [" + name + "]");
        addreses.remove(name);
        DelClient(name);
    }

    else if(message == " /FILE/ ")
    {
        QString txt, filter, file_text;
        QPixmap pix(500, 500);

        in >> txt;
        in >> filter;

        if(filter == "png") in >> pix;
        if(filter == "txt") in >> file_text;

        if(!dir.exists(QApplication::applicationDirPath() + "/Downloads"))
            dir.mkdir("Downloads");

        if(download->isChecked())
        {
            srand(time(NULL));

            if(filter == "png") pix.save(QApplication::applicationDirPath() + "/Downloads/" + QString::number(rand() % 1000), "PNG");

            if(filter == "txt")
            {
                QFile file(QApplication::applicationDirPath() + "/Downloads/" + QString::number(rand() % 1000));
                file.open(QIODevice::WriteOnly);
                QByteArray arr = file_text.toLatin1();
                file.write(arr);
                file.close();
            }

        }

        text->append(name + ": " + txt + message);

        QMap<QString, QHostAddress>::iterator it = addreses.begin();
        for(int i=0; i<addreses.size(); i++) // sending file from client to other all cliens (broadcast)
        {
            if(it.key() == name) continue;

            server->writeDatagram(datagram, it.value(), 2424);
            it++;
        }
    }
    else
    {
        text->append("[" + adr.toString() + "] " + name + ": " + message);

        QMap<QString, QHostAddress>::iterator it = addreses.begin();
        for(int i=0; i<addreses.size(); i++) // sending message from client to other all cliens (broadcast)
        {
            server->writeDatagram(datagram, it.value(), 2424);
            it++;
        }
    }

    Monitor();
}
