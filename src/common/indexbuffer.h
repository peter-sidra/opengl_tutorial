#pragma once

class IndexBuffer {
  private:
	// The id of the vbo, we're calling it renderer id to keep it generic with
	// other graphics APIs
	unsigned int m_rendererID;
	unsigned int m_count;
	bool moved = false;

  public:
	IndexBuffer(const unsigned int *data, unsigned int count);

	IndexBuffer(const IndexBuffer &other) = delete;
	IndexBuffer(IndexBuffer &&other);

	IndexBuffer operator=(const IndexBuffer &other) = delete;
	IndexBuffer &operator=(IndexBuffer &&other);

	~IndexBuffer();

	void bind() const;
	void unbind() const;

	[[nodiscard]] inline unsigned int GetCount() const {
		return m_count;
	};
};