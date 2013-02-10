#include "../src/Application.h"
#include "../src/Callbacks.h"
#include "../src/Window.h"
#include "../src/Canvas.h"
#include "../../libimage/src/image.h"

#include "../die-tk-extra/die-tk-extra.h"

#include <iostream>

int main()
{
	{
		using namespace tk;
		Application app;

		app.showConsole();

		auto window1 = app.createWindow();
		window1->onClose([]() -> bool {
			std::cout << "window was closed!" << std::endl;
			return true;
		});

		auto window2 = app.createWindow(WindowParams().text("second window"));
		window2->clear(RGBColor(100,0,0));
		window2->onClose([]() -> bool {
			static bool x = false;
			if( x ) {
				std::cout << "second window was closed!" << std::endl;
				return true;
			}
			x = true;
			std::cout << "try again!" << std::endl;
			return false;
		});


		window1->canvas().setBrush(tk::RGBColor(0,100,0));
		window1->canvas().setPen(tk::RGBColor(0,100,0));
		window1->onPaint([&](Canvas & canvas, Rect rect) {
			canvas.fillRect(rect);
		});
		auto button1 = app.createButton(window1,ControlParams().text("click me").start(20,20));
        button1->onClick([]() {
			std::cout << "clicked me" << std::endl;
		});

		button1->onMouse([](MouseEvent evt, Point pt) {
			std::cout << "clicked me with " << evt.type << " at " << pt << std::endl;
		});

		img::Image image("DIEGO1.jpg");
		auto imagepb = app.createPaintBox(window1,ControlParams().start(400,100).dims(200,200));
		imagepb->onPaint([&](Canvas & canvas, Rect rect) {
			canvas.drawImage(ImageRef::native(image.getWindowSystemHeader(),image.rawBits()));
		});
        
		auto paintboxG = app.createPaintBox(window2,ControlParams().start(window2->width()/2,window2->height()/2).dims(100,100));
		paintboxG->setBackground(tk::RGBColor(200,100,0));

		auto imageCtl = app.createImage(window2,ControlParams().start(300,100));
		imageCtl->setImage(image.rawBits(),image.getWindowSystemHeader());

		auto paintboxB = app.createPaintBox(window2,ControlParams().start(280,80).dims(100,100));
		paintboxB->setBackground(tk::RGBColor(0,100,200));

		auto buttonLeft = app.createButton(window2,ControlParams().text("left!").start(260,130));
		auto buttonRight = app.createButton(window2,ControlParams().text("right!").start(900,130));

        paintboxG->bringToFront();      // that one is hidden by the image control
        paintboxB->bringToFront();      // that is partially hidden

        buttonRight->bringToFront();

        auto combobox = app.createComboBox(window2,ControlParams().start(10,10));
        combobox->setVisibleItems(5);
        combobox->addString("laranja");
        combobox->addString("banana");
        combobox->addString("maca");
        combobox->addString("uva");
        combobox->addString("bergamota");
        combobox->addString("morango");
        combobox->addString("maracujeba");
        combobox->addString("apricu");
        
        combobox->onChange([&](){
			std::cout << "escolhi " << combobox->getString(combobox->selectedIndex()) << std::endl;
        });

        auto checkbox = app.createCheckBox(window2,ControlParams().start(100,100).text("disable box").autosize(true));
        checkbox->onClick([&](){
			combobox->setEnabled(! checkbox->checked());
        });

        auto imageList = app.createImageList(WDims(16,16));
        auto iBubble = addFile(*imageList,"bubble.png");
        auto iFolder = addFile(*imageList,"folder.png");
        
        auto treeView = app.createTreeView(window1,ControlParams().start(10,100).dims(100,300));
        treeView->setImageList(imageList);
        auto root = treeView->root();
        auto it1 = root.addChild(TreeView::ItemProperties().setText("test").setImageIndex(iFolder));
        auto item1 = *it1;
        item1.addChild(TreeView::ItemProperties().setText("child").setImageIndex(iBubble));
        item1.addChild(TreeView::ItemProperties().setText("child2"));
        root.addChild(TreeView::ItemProperties().setText("second item").setImageIndex(iFolder));
        root.addChild(TreeView::ItemProperties().setText("third item").setImageIndex(iFolder));
        
        treeView->onClick([&](){
			std::cout << "cliquei na treeview" << std::endl;
        });
        
        treeView->onChange([](TreeView::Item item){
			std::cout << "mudou para " << item.getProperties().text << std::endl;
        });
        
        treeView->beforeChange([](TreeView::Item before, TreeView::Item after) -> bool {
            return after.getProperties().text != "second item";
        });
        
        treeView->beforeCollapse([](TreeView::Item item) -> bool {
            std::cout << "vai encolher " << item.getProperties().text << std::endl;
            return true;
        });
        
		while( window1->state() == ws_visible || window2->state() == ws_visible ) {
			app.waitForMessages();
			app.processMessages();
		}
	}

	std::cout << "type something and press enter to quit ";
	int a;
	std::cin >> a;
}
