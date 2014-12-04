#include <algorithm>
#include <cstdint>
#include "Common/cpuinfo.h"
#include "Common/except.h"
#include "colorspace_param.h"
#include "matrix3.h"
#include "operation.h"
#include "operation_impl.h"
#include "operation_impl_x86.h"

namespace zimg {;
namespace colorspace {;

namespace {;

class PixelAdapterC : public PixelAdapter {
public:
	void f16_to_f32(const uint16_t *src, float *dst, int width) const override
	{
		throw ZimgUnsupportedError{ "f16 not supported in C impl" };
	}

	void f16_from_f32(const float *src, uint16_t *dst, int width) const override
	{
		throw ZimgUnsupportedError{ "f16 not supported in C impl" };
	}
};

class MatrixOperationC : public MatrixOperationImpl {
public:
	explicit MatrixOperationC(const Matrix3x3 &m) : MatrixOperationImpl(m)
	{}

	void process(float * const *ptr, int width) const override
	{
		for (int i = 0; i < width; ++i) {
			float a, b, c;
			float x, y, z;

			a = ptr[0][i];
			b = ptr[1][i];
			c = ptr[2][i];

			x = m_matrix[0][0] * a + m_matrix[0][1] * b + m_matrix[0][2] * c;
			y = m_matrix[1][0] * a + m_matrix[1][1] * b + m_matrix[1][2] * c;
			z = m_matrix[2][0] * a + m_matrix[2][1] * b + m_matrix[2][2] * c;

			ptr[0][i] = x;
			ptr[1][i] = y;
			ptr[2][i] = z;
		}
	}
};

class Rec709GammaOperationC : public Operation {
public:
	void process(float * const *ptr, int width) const override
	{
		for (int p = 0; p < 3; ++p) {
			for (int i = 0; i < width; ++i) {
				float x = ptr[p][i];

				ptr[p][i] = rec_709_gamma(x);
			}
		}
	}
};

class Rec709InverseGammaOperationC : public Operation {
public:
	void process(float * const *ptr, int width) const override
	{
		for (int p = 0; p < 3; ++p) {
			for (int i = 0; i < width; ++i) {
				float x = ptr[p][i];

				ptr[p][i] = rec_709_inverse_gamma(x);
			}
		}
	}
};

class Rec2020CLToRGBOperationC : public Operation {
public:
	void process(float * const *ptr, int width) const override
	{
		float kr = (float)REC_2020_KR;
		float kb = (float)REC_2020_KB;
		float kg = 1.0f - kr - kb;

		float pb = 0.7909854f;
		float nb = -0.9701716f;
		float pr = 0.4969147f;
		float nr = -0.8591209f;

		for (int i = 0; i < width; ++i) {
			float y = ptr[0][i];
			float u = ptr[1][i];
			float v = ptr[2][i];

			float r, g, b;
			float b_minus_y, r_minus_y;

			if (u < 0)
				b_minus_y = u * 2.0f * -nb;
			else
				b_minus_y = u * 2.0f * pb;

			if (v < 0)
				r_minus_y = v * 2.0f * -nr;
			else
				r_minus_y = v * 2.0f * pr;

			b = rec_709_inverse_gamma(b_minus_y + y);
			r = rec_709_inverse_gamma(r_minus_y + y);

			y = rec_709_inverse_gamma(y);
			g = (y - kr * r - kb * b) / kg;

			ptr[0][i] = r;
			ptr[1][i] = g;
			ptr[2][i] = b;
		}
	}
};

class Rec2020CLToYUVOperationC : public Operation {
public:
	void process(float * const *ptr, int width) const override
	{
		float kr = (float)REC_2020_KR;
		float kb = (float)REC_2020_KB;
		float kg = 1.0f - kr - kb;

		float pb = 0.7909854f;
		float nb = -0.9701716f;
		float pr = 0.4969147f;
		float nr = -0.8591209f;

		for (int i = 0; i < width; ++i) {
			float r = ptr[0][i];
			float g = ptr[1][i];
			float b = ptr[2][i];

			float y = rec_709_gamma(kr * r + kg * g + kb * b);
			float u, v;

			b = rec_709_gamma(b);
			r = rec_709_gamma(r);

			if (b - y < 0.0f)
				u = (b - y) / (2.0f * -nb);
			else
				u = (b - y) / (2.0f * pb);

			if (r - y < 0.0f)
				v = (r - y) / (2.0f * -nr);
			else
				v = (r - y) / (2.0f * pr);

			ptr[0][i] = y;
			ptr[1][i] = u;
			ptr[2][i] = v;
		}
	}
};

} // namespace


MatrixOperationImpl::MatrixOperationImpl(const Matrix3x3 &m)
{
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			m_matrix[i][j] = (float)m[i][j];
		}
	}
}

PixelAdapter *create_pixel_adapter(CPUClass cpu)
{
	PixelAdapter *ret = nullptr;
#ifdef ZIMG_X86
	ret = create_pixel_adapter_x86(cpu);
#endif
	if (!ret)
		ret = new PixelAdapterC{};

	return ret;
}

Operation *create_matrix_operation(const Matrix3x3 &m, CPUClass cpu)
{
	Operation *ret = nullptr;
#ifdef ZIMG_X86
	ret = create_matrix_operation_x86(m, cpu);
#endif
	if (!ret)
		ret = new MatrixOperationC{ m };

	return ret;
}

Operation *create_rec709_gamma_operation(CPUClass cpu)
{
	Operation *ret = nullptr;
#ifdef ZIMG_X86
	ret = create_rec709_gamma_operation_x86(cpu);
#endif
	if (!ret)
		ret = new Rec709GammaOperationC{};

	return ret;
}

Operation *create_rec709_inverse_gamma_operation(CPUClass cpu)
{
	Operation *ret = nullptr;
#ifdef ZIMG_X86
	ret = create_rec709_inverse_gamma_operation_x86(cpu);
#endif
	if (!ret)
		ret = new Rec709InverseGammaOperationC{};

	return ret;
}

Operation *create_2020_cl_yuv_to_rgb_operation(CPUClass cpu)
{
	return new Rec2020CLToRGBOperationC{};
}

Operation *create_2020_cl_rgb_to_yuv_operation(CPUClass cpu)
{
	return new Rec2020CLToYUVOperationC{};
}

} // namespace colorspace
} // namespace zimg
