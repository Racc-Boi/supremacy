#include "../../../includes.h"

void Form::update_opacity( ) {
	// opacity should reach 1 in 500 milliseconds.
	constexpr float frequency = 1.f / 0.5f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// if open		-> increment
	// if closed	-> decrement
	m_open ? m_opacity += step : m_opacity -= step;

	// clamp the opacity.
	math::clamp( m_opacity, 0.f, 1.f );

	m_alpha = 0xff * m_opacity;
}

void Form::draw_background( int x, int y, int width, int height, Color color ) {
	// draw background.
	render::rect_filled( x, y, width, height, color );

	// border colors.
	static const std::array<Color, 5> border_colors = {
		Color{5, 5, 5, m_alpha},
		Color{60, 60, 60, m_alpha},
		Color{40, 40, 40, m_alpha},
		Color{40, 40, 40, m_alpha},
		Color{60, 60, 60, m_alpha}
	};

	// draw border.
	for ( size_t i = 0; i < border_colors.size( ); ++i ) {
		render::rect( x + i, y + i, width - 2 * i, height - 2 * i, border_colors[ i ] );
	}
}

void Form::draw_tabs( ) {
	// check if we have any tabs.
	if ( m_tabs.empty( ) ) return;

	// get the tabs area.
	Rect tabs_area = GetTabsRect( );

	// draw the background.
	draw_background( tabs_area.x, tabs_area.y, tabs_area.w, tabs_area.h, { 17, 17, 17, m_alpha } );

	Color color = g_gui.m_color;
	color.a( ) = m_alpha;

	// iterate tabs.
	for ( size_t i = 0; i < m_tabs.size( ); ++i ) {
		// get the tab.
		const auto& t = m_tabs[ i ];

		// draw the tab title.
		render::menu_shade.string( tabs_area.x + 10, tabs_area.y + 5 + ( i * 16 ),
								   t == m_active_tab ? color : Color{ 152, 152, 152, m_alpha },
								   t->m_title );
	}
}

void Form::draw_elements( ) {
	// check if we have an active tab.
	if ( m_active_tab->m_elements.empty( ) ) return;

	// elements background and border.
	Rect el = GetElementsRect( );

	// draw the background.
	draw_background( el.x, el.y, el.w, el.h, { 17, 17, 17, m_alpha } );

	// draw the date and username.
	std::string date = XOR( __DATE__ );
	std::string text = tfm::format( XOR( "%s | %s" ), date.c_str( ), g_cl.m_user );
	render::menu_shade.string( el.x + el.w - 5, el.y + el.h - 16, { 205, 205, 205, m_alpha }, text, render::ALIGN_RIGHT );

	// iterate elements.
	for ( const auto& e : m_active_tab->m_elements ) {
		// skip elements that are not to be drawn.
		if ( !e || ( m_active_element && e == m_active_element ) || !e->m_show || !( e->m_flags & ElementFlags::DRAW ) )
			continue;

		// draw the element.
		e->draw( );
	}

	// draw the active element last.
	if ( m_active_element && m_active_element->m_show )
		m_active_element->draw( );
}

void Form::draw( ) {
	update_opacity( );
	if ( !m_alpha ) return;

	draw_background( m_x, m_y, m_width, m_height, { 12, 12, 12, m_alpha } );

	draw_tabs( );
	draw_elements( );
}
