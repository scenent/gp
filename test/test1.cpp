#include <gp.hpp>

#define OLC_PGE_APPLICATION
#include <test/olcPixelGameEngine.h>

class Demo1 : public olc::PixelGameEngine {
private:
	std::vector<gp::vec2> m_vertices{};
public:
	Demo1() = default;
	~Demo1() = default;
public:
	bool OnUserCreate() override {
		gp::GraphParser _gp{};
		m_vertices = _gp.exec("sin(x / 10.0) * 50.0 + 100", 0.0f, 500.0f, 0.5f, {});
		//m_vertices = _gp.exec("(x / 10.0) ^ 2 + 100", 0.0f, 500.0f, 0.5f, {});
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		this->Clear(olc::Pixel(0, 0, 0));
		for (size_t i = 0; i < m_vertices.size() - 1; i++) {
			this->DrawLine(
				olc::vi2d(static_cast<int>(m_vertices[i].x), static_cast<int>(m_vertices[i].y)),
				olc::vi2d(static_cast<int>(m_vertices[i + 1].x), static_cast<int>(m_vertices[i + 1].y))
			);
		}
		return true;
	}
};

int main() {
	Demo1 _demo;
	if (_demo.Construct(1024, 600, 1, 1)) {
		_demo.Start();
	}
}