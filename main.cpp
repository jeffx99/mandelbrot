#include <amp.h> // HELP ME! What is this 'AMP' thing? -- Here: https://msdn.microsoft.com/en-us/library/hh265136.aspx
#include <iostream>
#include <complex>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include "util_amp.hpp"
using namespace sf;
using namespace concurrency;

#define WIDTH 256
#define HEIGHT 256
#define MAX_ITERATIONS 1500

// With AMP
void mandelbrot_amp(index<2> idx, array_view<ampVertex, 2> all_points_amp, array_view<int, 1> iteration_counts, ampVector2d center, ampVector2d size, int least_iterations) restrict(amp) {
	ampVector2d z;
	z.x = center.x - 0.5 * size.x + (double(idx[1]) / double(WIDTH - 1)) * size.x;
	z.y = center.y + 0.5 * size.y - (double(idx[0]) / double(HEIGHT - 1)) * size.y;
	
	int m_iterations = 0;
	ampVector2d z_n = z;
	double xy = z.x * z.y, x2 = z.x * z.x, y2 = z.y * z.y;
	
	// Cardioid and Circle Check
	double ls = z.x + 1.0, rs = z.x - 0.25, rs2 = rs * rs;
	double cardioid_check = 4 * (rs2 + y2 + 0.5 * rs) * (rs2 + y2 + 0.5 * rs) - rs2 - y2;
	double circle_check = ls * ls + y2 - 0.0625;
	
	if (cardioid_check <= 0.0 || circle_check <= 0.0) {
		m_iterations = MAX_ITERATIONS;
	} 
	while (m_iterations < MAX_ITERATIONS && (x2 + y2 < 4.0)) {
		// Two iterations in one. Better??
		z_n.x = (x2 - y2) + z.x;
		z_n.y = (2.0 * xy) + z.y;

		x2 = z_n.x * z_n.x;
		y2 = z_n.y * z_n.y;
		xy = z_n.x * z_n.y;


		++m_iterations;
	}
	

	float fracl = (m_iterations - least_iterations) / float(MAX_ITERATIONS - least_iterations) - 1.0f;
	float fac = (1.0f + (fracl * fracl * fracl));
	float hue = 720.0f * fac;
	float sat = 1.0f - fac;
	// Create Point for screen
	all_points_amp[idx].position.x = idx[1];
	all_points_amp[idx].position.y = idx[0];

	all_points_amp[idx].color = amp_hsv_to_rgb(hue, sat, 1.0f);
	iteration_counts[idx[0] * WIDTH + idx[1]] =  m_iterations;
}

int main() {
	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Mandlebrot by Jeff");
	
	// COOL LOCATIONS
	// -0.789374599271466936740382412558 +0.163089252677526719026415054868i fabulous mandelbrot, julia set
	// -1.778103341943622395 + 0.007673942840457743i spiky mandelbrot
	// -1.295189082147777 + 0.440936982678320i brain clouud
	// -0.1592,-1.0317
	Vector2<double> center(-0.7893745992714669, 0.163089252677526719);
	Vector2<double> size(4, 4);

	ampVector2d amp_center, amp_size;
	
	// array of iteration counts for least_iteration purposes and an array_view later
	std::vector<int> iteration_counts(WIDTH * HEIGHT);
	// Old way was to actually use:
	std::vector<Vertex> all_points(WIDTH * HEIGHT);
	// NOW, we must use an amp allowed version
	std::vector<ampVertex> all_points_amp(WIDTH * HEIGHT);


	// AMP array_views
	array_view<ampVertex, 2> all_points_amp_av(HEIGHT, WIDTH, all_points_amp);
	array_view<int, 1> iteration_counts_av(WIDTH * HEIGHT, iteration_counts);
	all_points_amp_av.discard_data();
	iteration_counts_av.discard_data();

	int least_iterations = 0;

	double zoom = pow(1.03, 1);
	int zoom_count = 1;
	while (window.isOpen()) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) window.close();
		}
		window.clear();
		size /= zoom;
		std::cout << zoom_count << std::endl;
		zoom_count++;

		amp_center.x = center.x;
		amp_center.y = center.y;

		amp_size.x = size.x;
		amp_size.y = size.y;

		parallel_for_each(
			// Compute Domain
			all_points_amp_av.extent,
			// Lambda Expression
		[=](index<2> idx) restrict(amp) {
			mandelbrot_amp(idx, all_points_amp_av, iteration_counts_av, amp_center, amp_size, least_iterations);
		}
		);

		// Synchronize
		iteration_counts_av.synchronize();
		all_points_amp_av.synchronize();

		least_iterations = MAX_ITERATIONS;

		for (int count : iteration_counts) {
			if (count < least_iterations)
				least_iterations = count;
		}

		for (int i = 0; i < all_points_amp.size(); ++i) {
			all_points[i].position = Vector2f(all_points_amp[i].position.x, all_points_amp[i].position.y);
			all_points[i].color = Color(all_points_amp[i].color.r, all_points_amp[i].color.g, all_points_amp[i].color.b);
		}

		// Old method
		//least_iterations = mandelbrot(all_points, center, size, least_iterations);
		window.draw(&all_points[0], all_points.size(), PrimitiveType::Points);
		window.display();
	}
}