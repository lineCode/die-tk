#include "../die-tk.h"

#include "../die-tk-extra/die-tk-extra.h"
#include "../../libimage/src/image.h"
#include <fileutils.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

std::ostream & printAllItems(std::ostream & os, tk::MenuItem const & item, int level = 0);

void showWindows()
{
    using namespace tk;
    Application app;

    app.showConsole();

    Window window1;
    window1.onClose([]() -> bool {
        std::cout << "window was closed!" << std::endl;
        return true;
    });

    Window window2(WindowParams().text("second window"));
    window2.clear(RGBColor(100,0,0));
    window2.onClose([]() -> bool {
        static bool x = false;      // I know, yuck
        if( x ) {
            std::cout << "second window was closed!" << std::endl;
            return true;
        }
        x = true;
        std::cout << "try again!" << std::endl;
        return false;
    });
    
    img::Image image("images/DIEGO1.jpg");
    img::Image imageBug("images/bug.png");
    auto imgBugBmp = image::create(image::Params(imageBug.getWindowSystemHeader(),imageBug.rawBits()));
    auto imgBugExt = image::create(image::Params(imageBug.getWindowSystemHeader(),imageBug.rawBits()).externalBuffer().forceNoTransparent());
    auto imgRaw = image::create(image::Params(image::Type::BGR,WDims(630,418),image.rawBits()));
    auto imgRawExt = image::create(image::Params(image::Type::BGR,WDims(630,418),image.rawBits()).externalBuffer());
    auto imgBugTransp = image::create(image::Params(imageBug.getWindowSystemHeader(),imageBug.rawBits()).transparentIndex(imageBug.getTransparentColor()));
    img::Image imageX("images/x.png");
    auto imgX = image::create(image::Params(imageX.getWindowSystemHeader(),imageX.rawBits()));
    
    image::Byte rgbbuffer[] = {
        255,255,255, 255,255,255, 255,255,255, 0,255,255, 255,255,255, 255,255,255, 255,255,255, 
        255,0,255, 255,255,255, 255,255,255, 0,255,255, 255,255,255, 255,255,255, 255,255,255, 
        0,255,255, 255,255,255, 255,255,255, 0,255,255, 255,255,255, 255,255,255, 255,255,255, 
        255,255,0, 255,255,255, 255,255,255, 0,255,255, 255,255,255, 255,255,255, 255,255,255, 
        0,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 
        255,0,255, 255,255,255, 255,255,255, 255,255,255, 255,0,0,   255,0,0,     255,0,0,     
        255,255,0, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 
        255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 
    };
    image::Byte graybuffer[] = {
           0, 255, 255, 255, 255, 255, 255,   0,
         255, 200, 200, 200, 200, 200, 200, 255,
         255, 200, 100, 200, 200, 100, 200, 255,
         255, 120, 200, 120, 120, 200, 120, 255,
         255, 120, 200, 120, 120, 200, 120, 255,
         255, 200, 120, 200, 200, 120, 200, 255,
         255, 200, 200, 120, 120, 200, 200, 255,
           0, 255, 255, 255, 255, 255, 255,   0,
    };
    auto imgRgb = image::create(image::Params(image::Type::RGB,WDims(7,8),rgbbuffer));
    auto imgGray = image::create(image::Params(image::Type::gray,WDims(8,8),graybuffer));
    
    Menu menu;
    auto firstItemIt = menu.root().addItem("First Option");
    auto secondItemIt = menu.root().addItem("Second");
    auto thirdItemIt = menu.root().addItem("Third");
    firstItemIt->setText("Fist"_dies);
    secondItemIt->addItem("heh");
    auto subMenu1 = thirdItemIt->addItem("sub menu one");
    auto subMenu2 = thirdItemIt->addItem("sub menu two");
    subMenu1->addItem("1");
    subMenu1->addItem("2");
    subMenu2->addItem("3");
    subMenu2->addItem("4")->onClick([&](){ app.showMessage("laaaaaaaast item"); });
    firstItemIt->onClick([&](){ app.showMessage("first item"); });
    
    img::Image imageGhost("images/ghost0.png");
    auto fourthItemIt = menu.root().addItem("Images");
    fourthItemIt->addItem(toImage(imageGhost));
    fourthItemIt->addItem(toImageExt(img::Image("images/bubble.png")));
    fourthItemIt->addItem(toImageExt(imageBug));
    fourthItemIt->addItem(imgBugExt);
    fourthItemIt->addItem(toImage(img::Image("images/ladyBugLeft.png")));
    menu.attach(window1);
    
    printAllItems(std::cout,menu.root()) << std::endl;

    auto & bugCanvas = imgBugBmp->beginDraw();
    bugCanvas.setPen(tk::RGBColor(200,0,0));
    bugCanvas.drawLine(Point(2,2),Point(30,30));
    bugCanvas.drawLine(Point(30,2),Point(2,30));
    imgBugBmp->endDraw();
    
    window2.canvas().setBrush(tk::RGBColor(0,100,0));
    window2.canvas().setPen(tk::RGBColor(0,100,0));    
    window2.onPaint([&](Canvas & canvas, Rect rect) {
        canvas.fillRect(rect);
        canvas.draw(imgRaw,Point(500,480));
        canvas.draw(imgRawExt,Point(600,520));
        
        canvas.draw(imgBugExt,Point(20,400));
        canvas.draw(imgBugExt,Rect(60,400,110,450));
        canvas.draw(imgBugBmp,Point(20,440));
        canvas.draw(imgBugBmp,Rect(60,460,110,510));
        canvas.draw(imgRgb,Point(20,480));
        canvas.draw(imgGray,Point(20,490));
        canvas.draw(imgRgb,Rect(20,520,75,575));
        canvas.draw(imgGray,Rect(20,580,83,643));
        
        canvas.draw(imgBugTransp,Rect(120,400,160,432));
        canvas.draw(imgBugTransp,Point(120,400));
        canvas.draw(imgX,Point(120,440));
        canvas.draw(imgX,Rect(130,440,161,471));
        
        auto it = menu.root().begin();
        std::advance(it,3);
        auto itsub = it->begin();
        canvas.draw(itsub->getProperties().image,Point(165,440));
    });
    
    die::NativeString savedFilename;
    Button button1(window1,ControlParams().text("save file").start(20,20));
    button1.onClick([&]() {
        auto files = window1.selectFile(SelectFile::save);
        if( files.empty() ) {
            std::cout << "selected no files" << std::endl;
        } else {
            savedFilename = files[0];
            std::cout << "selected file " << savedFilename << std::endl;
        }
    });
    button1.onMouse([](MouseEvent evt, Point pt) {
        std::cout << "clicked me with " << evt.type << " at " << pt << std::endl;
    });
    
    Button buttonLF(window1,ControlParams().text("load file").start(button1.pos().addY(20)));
    buttonLF.onClick([&]() {
        auto files = window1.selectFile(SelectFile::load,SelectFileParams()
            .title("new title to load file")
            .showHidden()
            .path("C:\\")
            .filename(savedFilename)
            .multiSelect()
            .filter("*.txt")
            .filter("*.h","C++ header file")
            .filter("*.cpp","C++ source file")
        );
        if( files.empty() ) {
            std::cout << "selected no files" << std::endl;
        } else {
            std::cout << "selected files: ";
            std::copy(files.begin(),files.end(),std::ostream_iterator<die::NativeString>(std::cout," "));
            std::cout << std::endl;
        }
    });

    
    PaintBox paintboxG(window2,ControlParams().start(window2.width()/2,window2.height()/2).dims(100,100));
    paintboxG.setBackground(tk::RGBColor(200,100,0));

    Image imageCtl(window2,ControlParams().start(300,100));
    imageCtl.setImage(toImage(image));

    PaintBox paintboxB(window2,ControlParams().start(280,80).dims(100,100));
    paintboxB.setBackground(tk::RGBColor(0,100,200));

    Button buttonLeft(window2,ControlParams().text("left!").start(260,130));
    Button buttonRight(window2,ControlParams().text("right!").start(900,130));

    paintboxG.bringToFront();      // that one is hidden by the image control
    paintboxB.bringToFront();      // that is partially hidden

    buttonRight.bringToFront();

    ComboBox combobox(window2,ControlParams().start(10,10));
    combobox.setVisibleItems(5);
    combobox.addString("laranja");
    combobox.addString("banana");
    combobox.addString("maca");
    combobox.addString("uva");
    combobox.addString("bergamota");
    combobox.addString("morango");
    combobox.addString("maracujeba");
    combobox.addString("apricu");

    combobox.onChange([&](){
        std::cout << "chose " << combobox.getString(combobox.selectedIndex()) << std::endl;
    });

    CheckBox checkbox(window2,ControlParams().start(100,100).text("disable box").autosize());
    checkbox.onClick([&](){
        combobox.setEnabled(! checkbox.checked());
    });
    
    

    ImageList imageList(WDims(16,16));
    auto iBubble = addFile(imageList,"images/bubble.png");
    auto iFolder = addFile(imageList,"images/folder.png");

    TreeView treeView(window1,ControlParams().start(10,100).dims(100,300));
    treeView.setImageList(imageList);
    auto root = treeView.root();
    auto it1 = root.addChild(ItemProperties().setText("test").setImageIndex(iFolder));
    auto item1 = *it1;
    item1.addChild(ItemProperties().setText("child").setImageIndex(iBubble));
    item1.addChild(ItemProperties().setText("child2"));
    root.addChild(ItemProperties().setText("second item").setImageIndex(iFolder));
    root.addChild(ItemProperties().setText("third item").setImageIndex(iFolder));

    treeView.onChange([](TreeView::Item item){
        std::cout << "mudou para " << item.getProperties().text.toUTF8() << std::endl;
    });

    treeView.beforeChange([](TreeView::Item before, TreeView::Item after) -> bool {
        return after.getProperties().text != "second item";
    });

    treeView.beforeCollapse([](TreeView::Item item) -> bool {
        std::cout << "vai encolher " << item.getProperties().text.toUTF8() << std::endl;
        return true;
    });


    TreeView treeView2(window1,ControlParams().start(treeView.pos().addX(150)).dims(300,300));
    treeView2.setImageList(imageList);
    FileTreeView ftv(treeView2);
    ftv.setBaseDir("c:\\");

    Memo text(window1,ControlParams().start(500,100).dims(300,390));

    TableView tableView(window1,ControlParams().start(810,100).dims(300,200));
    tableView.addColumn(ColumnProperties("column 1",100));
    tableView.addColumn("column 2"_dies);
    tableView.addColumn("column 3"_dies);
    tableView.setColumn(1,ColumnProperties("second",30));

    auto colProp = tableView.column(2);
    std::cout << "column 2 width[" << colProp.width << "] header [" << colProp.header << "]" << std::endl;

    tableView.setImageList(imageList);
    tableView.addColumn(ColumnProperties("col image",100,1));
    tableView.addRow({die::NativeString("test")});
    tableView.addRow({"1"_dies,"2"_dies,"3"_dies});
    tableView.addRow({ItemProperties().setText("image").setImageIndex(1)});
    tableView.setItem(3,0,ItemProperties().setText("changed").setImageIndex(0));

    std::cout << "tableView selected row before " << tableView.selectedRow();
    tableView.setRowSelect(true);
    tableView.selectRow(1);
    std::cout << " after " << tableView.selectedRow() << std::endl;
    
    Button checkSelected(window1,ControlParams().start(tableView.pos().addY(-20)).text("check selected"_dies).autosize());
    checkSelected.onClick([&](){
        std::cout << "selected row " << tableView.selectedRow() << std::endl;
    });

    Label tvLabelRows(window1,ControlParams().text("set rows").start(tableView.rect().posDown(2)).autosize());
    Edit tvEditRows(window1,ControlParams().start(tvLabelRows.rect().posRight(2)).dims(30,20));
    Button tvButRows(window1,ControlParams().text("OK").start(tvEditRows.rect().posRight(2)).autosize());
    tvButRows.onClick([&](){
        tableView.setRows(tvEditRows.getText().toInt());
    });

    Label tvLabelDelRow(window1,ControlParams().text("delete row").start(tvLabelRows.rect().posDown(2)).autosize());
    Edit tvEditDelRow(window1,ControlParams().start(tvLabelDelRow.rect().posRight(2)).dims(30,20));
    Button tvButDelRow(window1,ControlParams().text("OK").start(tvEditDelRow.rect().posRight(2)).autosize());
    tvButDelRow.onClick([&](){
        tableView.deleteRow(tvEditDelRow.getText().toInt());
    });

    bool hasGrid = true;
    Button tvToggleGrid(window1,ControlParams().text("toggle grid lines").start(tvLabelDelRow.rect().posDown(2)).autosize());
    tvToggleGrid.onClick([&](){
        hasGrid = ! hasGrid;
        tableView.setGridLines(hasGrid);
    });

    Button buttonFileTreeView(window1,ControlParams()
        .start(treeView2.pos().addY(treeView2.height()+5))
        .autosize()
        .text("open"));
    buttonFileTreeView.onClick([&](){
        std::cout << "opening filename = " << ftv.getFile().filename << ' ' <<
                "full path = " << ftv.getFile().fullPath << ' ' <<
                "isDir? = " << ftv.getFile().isDirectory << std::endl;
        if( ftv.getFile().isDirectory ) return;

        fs::FileStreamWrapper file(ftv.getFile().fullPath);
        if( file ) {
            std::string buf(2048,'\0');
            file.read(&buf[0],buf.size());
            buf.resize(file.gcount());
            text.setText(buf);
        } else {
            std::cout << "fail!" << std::endl;
            app.showMessage("failed to open " + ftv.getFile().fullPath);
        }
    });

    PaintBox imagepb(window1,ControlParams().start(500,500).dims(200,200));
    imagepb.onPaint([&](Canvas & canvas, Rect rect) {
        canvas.draw(toImageExt(image),imagepb.rect().move(Point()));
    });


    while( window1.state() == ws_visible || window2.state() == ws_visible ) {
        app.waitForMessages();
        app.processMessages();
    }
}

std::ostream & printAllItems(std::ostream & os, tk::MenuItem const & item, int level)
{
    int i = 0;
    for( auto const & menuItem : item ) {
        os << level << "," << i++ << ") ";
        auto const & prop = menuItem.getProperties();
        if( prop.hasImage() ) {
            os << "image dims " << prop.image->dims() << " ";
        } else {
            os << prop.text << " ";
        }
        printAllItems(os,menuItem,level+1);
    }
    return os;
}


int main()
{
    showWindows();
    
	std::cout << "type something and press enter to quit ";
	int a;
	std::cin >> a;
}

