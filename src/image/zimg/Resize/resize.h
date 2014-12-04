#pragma once

#ifndef ZIMG_RESIZE_RESIZE_H_
#define ZIMG_RESIZE_RESIZE_H_

#include <cstddef>
#include <memory>

namespace zimg {;

enum class CPUClass;
enum class PixelType;

template <class T>
class ImagePlane;

namespace resize {;

class Filter;
class ResizeImpl;

/**
 * Resize: applies a resizing filter.
 *
 * Each instance is applicable only for its given set of resizing parameters.
 */
class Resize {
	int m_src_width;
	int m_src_height;
	int m_dst_width;
	int m_dst_height;
	bool m_skip_h;
	bool m_skip_v;
	std::shared_ptr<ResizeImpl> m_impl;

	size_t max_frame_size(PixelType type) const;

	void invoke_impl_h(const ImagePlane<const void> &src, const ImagePlane<void> &dst, void *tmp) const;

	void invoke_impl_v(const ImagePlane<const void> &src, const ImagePlane<void> &dst, void *tmp) const;
public:
	/**
	 * Initialize a null context. Cannot be used for execution.
	 */
	Resize() = default;

	/**
	 * Initialize a context to apply a given resizing filter.
	 *
	 * @param f filter
	 * @param src_width width of input image
	 * @param src_height height of input image
	 * @param dst_width width of output image
	 * @param dst_height height of output image
	 * @param shift_w horizontal shift in units of source pixels
	 * @param shift_h vertical shift in units of source pixels
	 * @param subwidth active horizontal subwindow in units of source pixels
	 * @param subheight active vertical subwindow in units of source pixels
	 * @param cpu create kernel optimized for given cpu
	 * @throws ZimgIllegalArgument on unsupported parameter combinations
	 * @throws ZimgOutOfMemory if out of memory
	 */
	Resize(const Filter &f, int src_width, int src_height, int dst_width, int dst_height,
	       double shift_w, double shift_h, double subwidth, double subheight, CPUClass cpu);

	/**
	 * Get the size of the temporary buffer required by the filter.
	 *
	 * @param type pixel type to process
	 * @return the size of temporary buffer in units of pixels
	 */
	size_t tmp_size(PixelType type) const;

	/**
	 * Process an image. The input and output pixel formats must match.
	 *
	 * @param src input plane
	 * @param dst output plane
	 * @param tmp temporary buffer (@see Resize::tmp_size)
	 * @throws ZimgUnsupportedError if pixel type not supported
	 */
	void process(const ImagePlane<const void> &src, const ImagePlane<void> &dst, void *tmp) const;
};

} // namespace resize
} // namespace zimg

#endif // ZIMG_RESIZE_RESIZE_H_
