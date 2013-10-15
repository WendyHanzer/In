#include <GL/glew.h>
#include <ImageMagick/Magick++.h>

class texLoader {
	public:
		texLoader(GLenum textureTarget, const std::string& fileName);
		bool load();
		void bind(GLenum textureUnit);
	private:
		std::string m_fileName;
		GLenum m_textureTarget;
		GLuint m_textureObj;
		Magick::Image* m_pImage;
		Magick::Blob m_blob;
};
