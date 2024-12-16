/**
 * @file
 * @brief Source file for Outline effect class
 * @author Jonathan Thomas <jonathan@openshot.org>, HaiVQ <me@haivq.com>
 *
 * @ref License
 */

// Copyright (c) 2008-2019 OpenShot Studios, LLC
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Outline.h"
#include "Exceptions.h"

using namespace openshot;

/// Blank constructor, useful when using Json to load the effect properties
Outline::Outline() : width(3.0), red(0.0), green(0.0), blue(0.0), alpha(255.0) {
	// Init effect properties
	init_effect_details();
}

// Default constructor
Outline::Outline(Keyframe width, Keyframe red, Keyframe green, Keyframe blue, Keyframe alpha) :
	width(width), red(red), green(green), blue(blue), alpha(alpha)
{
	// Init effect properties
	init_effect_details();
}

// Init effect settings
void Outline::init_effect_details()
{
	/// Initialize the values of the EffectInfo struct.
	InitEffectInfo();

	/// Set the effect info
	info.class_name = "Outline";
	info.name = "Outline";
	info.description = "Add outline around the image with transparent background.";
	info.has_audio = false;
	info.has_video = true;
}

// This method is required for all derived classes of EffectBase, and returns a
// modified openshot::Frame object
std::shared_ptr<openshot::Frame> Outline::GetFrame(std::shared_ptr<openshot::Frame> frame, int64_t frame_number)
{
	// Get the frame's image
	std::shared_ptr<QImage> frame_image = frame->GetImage();

	int sigmaValue = width.GetValue(frame_number) / 3;
	int redValue = red.GetValue(frame_number);
	int greenValue = green.GetValue(frame_number);
	int blueValue = blue.GetValue(frame_number);
	int alphaValue = alpha.GetValue(frame_number);
	
	// Get BGRA image from QImage
	cv::Mat cv_image = QImageToBGRACvMat(frame_image);

	// extract alpha channel to create the alpha mask from the image
	std::vector<cv::Mat> channels(4);
    cv::split(cv_image, channels);
	cv::Mat alpha_mask = channels[3].clone();
	
	// Disable de-antialiased
	// cv::threshold(alpha_mask, alpha_mask, 254, 255, cv::ThresholdTypes::THRESH_BINARY); // threshold the alpha channel to remove aliased edges

	
	// Create the outline mask
	cv::Mat outline_mask;
	cv::GaussianBlur(alpha_mask, outline_mask, cv::Size(0, 0), sigmaValue, sigmaValue, cv::BorderTypes::BORDER_DEFAULT);
	cv::threshold(outline_mask, outline_mask, 0, 255, cv::ThresholdTypes::THRESH_BINARY);

	// Antialias the outline edge
	// Apply Canny edge detection to the outline mask
	cv::Mat edge_mask;
	cv::Canny(outline_mask, edge_mask, 250, 255);

	// Apply Gaussian blur only to the edge mask
	cv::Mat blurred_edge_mask;
	cv::GaussianBlur(edge_mask, blurred_edge_mask, cv::Size(0, 0), 0.8, 0.8, cv::BorderTypes::BORDER_DEFAULT);
	
	// Combine the blurred edge mask with the original alpha mask
	cv::Mat combined_mask;
	cv::bitwise_or(outline_mask, blurred_edge_mask, outline_mask);
	
	cv::Mat final_image;

	// create solid color source mat
	cv::Mat solid_color_mat(cv::Size(cv_image.cols, cv_image.rows), CV_8UC4, cv::Scalar(blueValue, greenValue, redValue, alphaValue));
	
	// place outline image first, then place the original image (de-antialiased) on top
	solid_color_mat.copyTo(final_image, outline_mask);
	cv_image.copyTo(final_image, alpha_mask);
	
	std::shared_ptr<QImage> new_frame_image = BGRACvMatToQImage(final_image);

	// FIXME: The shared_ptr::swap does not work somehow
	// frame_image.swap(new_frame_image);
	*frame_image = *new_frame_image;
	
	
	// return the modified frame
	return frame;
}

cv::Mat Outline::QImageToBGRACvMat(std::shared_ptr<QImage>& qimage) {
	return cv::Mat(qimage->height(), qimage->width(), CV_8UC4, (uchar*)qimage->constBits(), qimage->bytesPerLine());
}

std::shared_ptr<QImage> Outline::BGRACvMatToQImage(cv::Mat img) {
	QImage qimage(img.data, img.cols, img.rows, img.step, QImage::Format_ARGB32);
	std::shared_ptr<QImage> imgIn = std::make_shared<QImage>(qimage.convertToFormat(QImage::Format_RGBA8888_Premultiplied));
	return imgIn;
}

// Generate JSON string of this object
std::string Outline::Json() const {

	// Return formatted string
	return JsonValue().toStyledString();
}

// Generate Json::Value for this object
Json::Value Outline::JsonValue() const {

	// Create root json object
	Json::Value root = EffectBase::JsonValue(); // get parent properties
	root["type"] = info.class_name;
	root["width"] = width.JsonValue();
	root["red"] = red.JsonValue();
	root["green"] = green.JsonValue();
	root["blue"] = blue.JsonValue();
	root["alpha"] = alpha.JsonValue();

	// return JsonValue
	return root;
}

// Load JSON string into this object
void Outline::SetJson(const std::string value) {

	// Parse JSON string into JSON objects
	try
	{
		const Json::Value root = openshot::stringToJson(value);
		// Set all values that match
		SetJsonValue(root);
	}
	catch (const std::exception& e)
	{
		// Error parsing JSON (or missing keys)
		throw InvalidJSON("JSON is invalid (missing keys or invalid data types)");
	}
}

// Load Json::Value into this object
void Outline::SetJsonValue(const Json::Value root) {

	// Set parent data
	EffectBase::SetJsonValue(root);

	// Set data from Json (if key is found)
	if (!root["width"].isNull())
		width.SetJsonValue(root["width"]);
	if (!root["red"].isNull())
		red.SetJsonValue(root["red"]);
	if (!root["green"].isNull())
		green.SetJsonValue(root["green"]);
	if (!root["blue"].isNull())
		blue.SetJsonValue(root["blue"]);
	if (!root["alpha"].isNull())
		alpha.SetJsonValue(root["alpha"]);
}

// Get all properties for a specific frame
std::string Outline::PropertiesJSON(int64_t requested_frame) const {

	// Generate JSON properties list
	Json::Value root = BasePropertiesJSON(requested_frame);

	// Keyframes
	root["width"] = add_property_json("Width", width.GetValue(requested_frame), "float", "", &width, 0, 1000, false, requested_frame);
	root["red"] = add_property_json("Red", red.GetValue(requested_frame), "float", "", &red, 0, 255, false, requested_frame);
	root["green"] = add_property_json("Green", green.GetValue(requested_frame), "float", "", &green, 0, 255, false, requested_frame);
	root["blue"] = add_property_json("Blue", blue.GetValue(requested_frame), "float", "", &blue, 0, 255, false, requested_frame);
	root["alpha"] = add_property_json("Alpha", alpha.GetValue(requested_frame), "float", "", &alpha, 0, 255, false, requested_frame);

	// Return formatted string
	return root.toStyledString();
}
