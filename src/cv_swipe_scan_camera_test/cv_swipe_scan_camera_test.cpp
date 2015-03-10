#include "stdafx.h"

std::string window_name = "cv_swipe_scan_camera_test";

cv::Mat capture_img;
cv::Mat slit_scan_img;
cv::Rect copy_roi;

bool show_capture_preview = true;
bool press_button_l = false;
int old_mouse_x;
int old_mouse_y;

cv::Rect correct_rect(const cv::Rect &src, const cv::Size &size)
{
	cv::Rect r = src;

	if (r.x < 0) r.x = 0;
	if (r.y < 0) r.y = 0;
	if (size.width - 1 < r.x) r.x = size.width - 1;
	if (size.height - 1 < r.y) r.y = size.height - 1;

	if (r.width <= 0) r.width = 1;
	if (r.height <= 0) r.height = 1;
	if (size.width < r.x + r.width) r.width = size.width - r.x;
	if (size.height < r.y + r.height) r.height = size.height - r.x;

	return r;
}

cv::Rect create_rect(const int &x0, const int &y0, const int &x1, const int &y1)
{
	cv::Rect r;
	if (x0 < x1) {
		r.x = x0;
		r.width = x1 - x0;
	}
	else {
		r.x = x1;
		r.width = x0 - x1;
	}

	if (y0 < y1) {
		r.y = y0;
		r.height = y1 - y0;
	}
	else {
		r.y = y1;
		r.height = y0 - y1;
	}

	r = correct_rect(r, slit_scan_img.size());

	return r;
}

void onMouse(int event, int x, int y, int, void*)
{
	if (capture_img.empty()) return;

	if (event == cv::EVENT_LBUTTONDOWN) {
		press_button_l = true;

		old_mouse_x = x;
		old_mouse_y = y;
	}
	else if (event == cv::EVENT_LBUTTONUP) {
		press_button_l = false;
	}

	if (press_button_l) {
		int diff_x = x - old_mouse_x;
		int diff_y = y - old_mouse_y;

		if (abs(diff_x) > 0) {
			 copy_roi = create_rect(old_mouse_x, 0, x, slit_scan_img.rows);
			 capture_img(copy_roi).copyTo(slit_scan_img(copy_roi));
		}
		else {
			copy_roi = cv::Rect(); // clear
		}

		old_mouse_x = x;
		old_mouse_y = y;
	}
}

void draw()
{
	cv::Mat canvas(slit_scan_img.size(), CV_8UC3);

	slit_scan_img.copyTo(canvas);

	if (press_button_l) {
		cv::rectangle(canvas, copy_roi, cv::Scalar(0, 0, 255), 1);
	}

	if (show_capture_preview) {
		cv::Rect capture_roi(cv::Point(0, 0), cv::Size(capture_img.cols / 4, capture_img.rows / 4));
		cv::Mat resized_capture_img;
		cv::resize(capture_img, resized_capture_img, capture_roi.size());
		resized_capture_img.copyTo(canvas(capture_roi));
		cv::rectangle(canvas, capture_roi, cv::Scalar(0, 255, 0), 1);
	}

	cv::imshow(window_name, canvas);
}

int main(int argc, char* argv[])
{
	cv::VideoCapture capture;

	bool rv = capture.open(0);
	if (rv == false) {
		std::cerr << "error : capture.open() failed..." << std::endl;
		return -1;
	}

	capture >> capture_img;
	slit_scan_img.create(capture_img.size(), CV_8UC3);
	slit_scan_img = 0;

	cv::namedWindow(window_name);
	cv::setMouseCallback(window_name, onMouse, NULL);

	while (true) {
		capture >> capture_img;

		if (!capture_img.empty()) {
			draw();
		}

		int c = cv::waitKey(1);
		if (c == 27) {
			break;
		}
		else if (c == 'c') {
			slit_scan_img = 0;
		}
		else if (c == 'p') {
			show_capture_preview = !show_capture_preview;
		}
	}

	capture.release();
	cv::destroyAllWindows();

	return 0;
}
