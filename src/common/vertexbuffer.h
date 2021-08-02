#pragma once

class VertexBuffer {
  private:
	// The id of the vbo, we're calling it renderer id to keep it generic with
	// other graphics APIs
	unsigned int m_rendererID;
	bool moved = false;

  public:
	VertexBuffer(const void *data, unsigned int size);

	VertexBuffer(const VertexBuffer &other) = delete;
	VertexBuffer(VertexBuffer &&other);

	VertexBuffer operator=(const VertexBuffer &other) = delete;
	VertexBuffer &operator=(VertexBuffer &&other);

	~VertexBuffer();

	void bind() const;
	void unbind() const;
};