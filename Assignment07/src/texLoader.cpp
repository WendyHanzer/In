#include <iostream>
#include "texLoader.h"

texLoader::texLoader(GLenum textureTarget, const std::string& fileName) {
	m_textureTarget = textureTarget;
	m_fileName = fileName;
	m_pImage = NULL;	
}

bool texLoader::load() {
	try {
		m_pImage = new Magick::Image(m_fileName);
		m_pImage->write(&m_blob, "RGBA");
	}
	catch(Magick::Error& Error) {
		std::cout << "Error loading the texture " << m_fileName << ": " << Error.what() << std::endl;
		return false;
	}
	
	glGenTextures(1, &m_textureObj);
	glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(m_textureTarget, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void texLoader::bind(GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
