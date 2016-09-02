#include <die-tk.h>
#include <convert.h>
#include <iostream>

using namespace tk;

int main()
{
	Application app;

	Window window(WindowParams("test image load and transparency").dims(500,400));

	auto boss = convertImage(img::Image("boss.png"));
	auto diego = convertImage(img::Image("DIEGO1.jpg"));
	auto explosion = convertImage(img::Image("explosion0.png"));
	auto explosion3 = convertImage(img::Image("explosion3.png"));
	auto bugLeft = convertImage(img::Image("bugLeft.png"));
	auto gameTitle = convertImage(img::Image("gameTitle.png"));
	window.onPaint([&](Canvas & canvas, Rect) {
		diego->drawInto(canvas,Point(0,0));
		explosion->drawInto(canvas,Point(5,5));
		explosion3->drawInto(canvas,Point(80,5));
		boss->drawInto(canvas,Point(20,100));
		bugLeft->drawInto(canvas,Point(300,5));
		gameTitle->drawInto(canvas,Point(200,40));
	});

	PaintBox p(window, ControlParams().start(5,40).dims(32,32));
	p.onPaint([&](Canvas & canvas, Rect rect) {
		std::cout << rect << std::endl;
		explosion->copyRectInto(canvas,rect,rect.topLeft());
	});
	// TODO make Image support transparent images - it is cloneBitmap()'s fault
	Image i(window, ControlParams().start(p.pos().addY(32)).autosize());
	i.setImage(explosion);
	Image j = i.clone();
	j.setBackground(RGBColor(120,0,0));
	j.setPos(i.pos().addX(32));
	j.setDims(WDims(40,40));

	do {
		app.waitForMessages();
		app.processMessages();
	} while( window.visible() );
}
