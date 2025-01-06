/**
 * @file
 * @brief Header file for Outline effect class
 * @author Jonathan Thomas <jonathan@openshot.org>, HaiVQ <me@haivq.com>
 *
 * @ref License
 */

// Copyright (c) 2008-2019 OpenShot Studios, LLC
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OPENSHOT_OUTLINE_EFFECT_H
#define OPENSHOT_OUTLINE_EFFECT_H

#include "../EffectBase.h"

#include "../Frame.h"
#include "../Json.h"
#include "../KeyFrame.h"

#include <memory>
#include <string>


namespace openshot
{

	/**
	 * @brief This class add the outline around image with transparent background and can be animated
	 * with openshot::Keyframe curves over time.
	 *
	 * Since outline effect is pretty useful in many cases, this effect is added to libopenshot.
	 */
	class Outline : public EffectBase
	{
	private:
		/// Init effect settings
		void init_effect_details();

		// Convert QImage to cv::Mat and vice versa
		// Although Frame class has GetImageCV, but it does not include alpha channel
		// so we need a separate methods which preserve alpha channel
		// Idea from: https://stackoverflow.com/a/78480103
		cv::Mat QImageToBGRACvMat(std::shared_ptr<QImage>& qimage);
		std::shared_ptr<QImage> BGRACvMatToQImage(cv::Mat img);

	public:
		Keyframe width;	///< Width of the outline
		Keyframe red;	///< Red channel of the outline
		Keyframe green;	///< Green of the outline
		Keyframe blue;	///< Blue of the outline
		Keyframe alpha;	///< Alpha of the outline

		/// Blank constructor, useful when using Json to load the effect properties
		Outline();

		/// Default constructor, which require width, red, green, blue, alpha
		///
		/// @param width the width of the outline (between 0 and 1000, rounded to int)
		/// @param red the red channel of the outline (between 0 and 255, rounded to int)
		/// @param green the green channel of the outline (between 0 and 255, rounded to int)
		/// @param blue the blue channel of the outline (between 0 and 255, rounded to int)
		/// @param alpha the alpha channel of the outline (between 0 and 255, rounded to int)
		Outline(Keyframe width, Keyframe red, Keyframe green, Keyframe blue, Keyframe alpha);

		/// @brief This method is required for all derived classes of ClipBase, and returns a
		/// new openshot::Frame object. All Clip keyframes and effects are resolved into
		/// pixels.
		///
		/// @returns A new openshot::Frame object
		/// @param frame_number The frame number (starting at 1) of the clip or effect on the timeline.
		std::shared_ptr<openshot::Frame> GetFrame(int64_t frame_number) override { return GetFrame(std::make_shared<openshot::Frame>(), frame_number); }

		/// @brief This method is required for all derived classes of ClipBase, and returns a
		/// modified openshot::Frame object
		///
		/// The frame object is passed into this method and used as a starting point (pixels and audio).
		/// All Clip keyframes and effects are resolved into pixels.
		///
		/// @returns The modified openshot::Frame object
		/// @param frame The frame object that needs the clip or effect applied to it
		/// @param frame_number The frame number (starting at 1) of the clip or effect on the timeline.
		std::shared_ptr<openshot::Frame> GetFrame(std::shared_ptr<openshot::Frame> frame, int64_t frame_number) override;

		// Get and Set JSON methods
		std::string Json() const override; ///< Generate JSON string of this object
		void SetJson(const std::string value) override; ///< Load JSON string into this object
		Json::Value JsonValue() const override; ///< Generate Json::Value for this object
		void SetJsonValue(const Json::Value root) override; ///< Load Json::Value into this object

		/// Get all properties for a specific frame (perfect for a UI to display the current state
		/// of all properties at any time)
		std::string PropertiesJSON(int64_t requested_frame) const override;
	};

}

#endif
