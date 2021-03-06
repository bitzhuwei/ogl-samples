///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2013 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/generate_mipmaps.inl
/// @date 2010-09-27 / 2013-01-12
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

namespace gli{
namespace detail
{

}//namespace detail

	template <typename genType>
	inline texture2D generate_mipmaps(texture2D const & Texture)
	{
		texture2D Result(Texture.format(), Texture.dimensions());

		for(texture2D::size_type Level = Texture.baseLevel(); Level < Texture.maxLevel(); ++Level)
		{
			// Src
			std::size_t BaseWidth = Result[Level].dimensions().x;
			genType* DataSrc = reinterpret_cast<genType*>(Result[Level + 0].data());

			// Dst
			texture2D::dim_type LevelDimensions = texture2D::dim_type(Result[Level].dimensions()) >> texture2D::dim_type(1);
			LevelDimensions = glm::max(LevelDimensions, texture2D::dim_type(1));
			genType* DataDst =  reinterpret_cast<genType*>(Result[Level + 1].data());

			for(std::size_t j = 0; j < LevelDimensions.y; ++j)
			for(std::size_t i = 0; i < LevelDimensions.x;  ++i)
			{
				std::size_t x = (i << 1);
				std::size_t y = (j << 1);

				std::size_t Index00 = ((x + 0) + (y + 0) * BaseWidth);
				std::size_t Index01 = ((x + 0) + (y + 1) * BaseWidth);
				std::size_t Index11 = ((x + 1) + (y + 1) * BaseWidth);
				std::size_t Index10 = ((x + 1) + (y + 0) * BaseWidth);

				genType const & Data00 = DataSrc[Index00];
				genType const & Data01 = DataSrc[Index01];
				genType const & Data11 = DataSrc[Index11];
				genType const & Data10 = DataSrc[Index10];

				std::size_t IndexDst = (x + y * LevelDimensions.x);

				*(DataDst + IndexDst) = (Data00 + Data01 + Data11 + Data10) / typename genType::value_type(4);
			}
		}

		return Result;
	}

}//namespace gli
