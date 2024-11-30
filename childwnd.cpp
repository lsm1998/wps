#include "childwnd.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextBlockFormat>
#include <QTextListFormat>
#include <QtWidgets>

ChildWnd::ChildWnd()
{
    //子窗口关闭时销毁该类的实例对象
    setAttribute(Qt::WA_DeleteOnClose);

    m_bSaved = false;
}

void ChildWnd::newDoc()
{
    static int wndSeqNum = 1;
    m_CurDocPath = QString("WPS 文档 %1")
                    .arg(wndSeqNum++);

    //设置窗体标题，文档改动后名称后加"*"号标识
    setWindowTitle(m_CurDocPath + "[*]" + " - MyWPS");
    connect(document(),SIGNAL(contentsChanged()),
            this,SLOT(docBeModified()));
}

QString ChildWnd::getCurDocName()
{
    return QFileInfo(m_CurDocPath).fileName();
}

bool ChildWnd::loadDoc(const QString &docName)
{
    if(!docName.isEmpty())
    {
        QFile file(docName);
        if(!file.exists())  return false;

        if(!file.open(QFile::ReadOnly)) return false;
        QByteArray text = file.readAll();
        if(Qt::mightBeRichText(text))
            setHtml(text);
        else
            setPlainText(text);
        setCurDoc(docName);
        connect(document(),SIGNAL(contentsChanged()),
                this,SLOT(docBeModified()));
        return true;
    }
}

void ChildWnd::setCurDoc(const QString &docName)
{
    //canonicalFilePath()返回标准名称路径，可以过滤"." ".."
    m_CurDocPath = QFileInfo(docName).canonicalFilePath();
    m_bSaved = true;                //文档已被保存
    document()->setModified(false); //文档未改动
    setWindowModified(false);       //窗口不显示改动标识
    setWindowTitle(getCurDocName() + "[*]");//设置子窗口标题
}

bool ChildWnd::saveDoc()
{
    if(m_bSaved) return saveDocOpt(m_CurDocPath);
    else saveAsDoc();
}

bool ChildWnd::saveAsDoc()
{
    QString docName = QFileDialog::getSaveFileName(this,
                                 "另存为",
                                 m_CurDocPath,
                                 "HTML文档 (*.htm *.html);;"
                                 "所有文件(*.*)");
    if(docName.isEmpty()) return false;
    else return saveDocOpt(docName);
}

bool ChildWnd::saveDocOpt(QString docName)
{
    if( !(docName.endsWith(".htm",Qt::CaseInsensitive) ||
          docName.endsWith(".html",Qt::CaseInsensitive)) ){
        docName += ".html";
    }

    QTextDocumentWriter writer(docName);
    bool isSuccess = writer.write(this->document());
    if(isSuccess) setCurDoc(docName);
    return isSuccess;
}

void ChildWnd::setFormatOnSelectedWord(const QTextCharFormat &fmt)
{
    QTextCursor tcursor = textCursor();
    if(!tcursor.hasSelection()) tcursor.select(QTextCursor::WordUnderCursor);

    tcursor.mergeCharFormat(fmt);
    mergeCurrentCharFormat(fmt);
}

void ChildWnd::setAlignOfDocumentText(int aligntype)
{
    if(aligntype == 1)
        setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if(aligntype == 2)
        setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if(aligntype == 3)
        setAlignment(Qt::AlignCenter);
    else if(aligntype == 4)
        setAlignment(Qt::AlignJustify);

}

void ChildWnd::setParaSyle(int pStyle)
{
    QTextCursor tcursor = textCursor();
    QTextListFormat::Style sname;
    if(pStyle != 0)
    {
        switch(pStyle){
        case 1:
            sname = QTextListFormat::ListDisc;//黑心实心圆
            break;
        case 2:
            sname = QTextListFormat::ListCircle;//空心圆
            break;
        case 3:
            sname = QTextListFormat::ListSquare;//方形
            break;
        case 4:
            sname = QTextListFormat::ListDecimal;//十进制整数
            break;
        case 5:
            sname = QTextListFormat::ListLowerAlpha;//小写字母
            break;
        case 6:
            sname = QTextListFormat::ListUpperAlpha;//大写字母
            break;
        case 7:
            sname = QTextListFormat::ListLowerRoman;//小写罗马字母
            break;
        case 8:
            sname = QTextListFormat::ListUpperRoman;//大写罗马字母
            break;
        default:
            sname = QTextListFormat::ListDisc;
        }

        tcursor.beginEditBlock();   //开始编辑文本块
        QTextBlockFormat tBlockFmt = tcursor.blockFormat(); //文本格式
        QTextListFormat tListFmt;   //标号格式
        if(tcursor.currentList()){
            tListFmt = tcursor.currentList()->format();
        }else{
            tListFmt.setIndent(tBlockFmt.indent() + 1);//设置缩进
            tBlockFmt.setIndent(0);
            tcursor.setBlockFormat(tBlockFmt);
        }

        tListFmt.setStyle(sname);   //设置标号风格
        tcursor.createList(tListFmt);

        tcursor.endEditBlock();     //结束编辑文本块
    }else{
        QTextBlockFormat tbfmt;
        tbfmt.setObjectIndex(-1);   //设置无效
        tcursor.mergeBlockFormat(tbfmt);
    }
}

void ChildWnd::closeEvent(QCloseEvent *event)
{
    if( promptSave() )
        event->accept();
    else
        event->ignore();
}

bool ChildWnd::promptSave()
{
    if(! document()->isModified()) return true;

    QMessageBox::StandardButton result;
    result = QMessageBox::warning(this,
                                  QString("系统提示"),
                                  QString("文档%1已修改，是否保存？")
                                  .arg(getCurDocName()),
                                  QMessageBox::Yes |
                                  QMessageBox::Discard |
                                  QMessageBox::No);
    if(result == QMessageBox::Yes)
        return saveDoc();
    else if(result == QMessageBox::No)
        return false;
    return true;

}

void ChildWnd::docBeModified()
{
    setWindowModified(document()->isModified());
}
