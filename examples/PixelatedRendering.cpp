#include "vulkan_engine.h"

class PixelatedRenderingApp : protected VulkanWindow {
	using VulkanWindow::VulkanWindow;
private:
	void init() {}
	void mainLoop() {}
	void cleanup() {}
public:
	void run() {
		init();
		mainLoop();
		cleanup();
	}

};

int main(int argc, char** argv) {
	VulkanEngine vk;
	PixelatedRenderingApp app(&vk);
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}