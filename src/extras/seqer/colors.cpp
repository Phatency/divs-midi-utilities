
#include <QtWidgets>
#include "colors.h"

QColor Colors::shade(QColor base_color, int light_theme_lightness, int dark_theme_lightness)
{
	int base_hue, base_saturation, base_lightness;
	base_color.getHsl(&base_hue, &base_saturation, &base_lightness);
	bool dark_theme = (QGuiApplication::palette().color(QPalette::Button).lightness() < 127);
	return QColor::fromHsl(base_hue, base_saturation, dark_theme ? dark_theme_lightness : light_theme_lightness);
}

QColor Colors::relativeShade(QColor base_color, float relative_lightness)
{
	int base_hue, base_saturation, base_lightness;
	base_color.getHsl(&base_hue, &base_saturation, &base_lightness);
	bool dark_theme = (QGuiApplication::palette().color(QPalette::Button).lightness() < 127);
	return QColor::fromHsl(base_hue, base_saturation, (int)(base_lightness * (dark_theme ? (1 / relative_lightness) : relative_lightness)));
}

QColor Colors::buttonShade(int light_theme_lightness, int dark_theme_lightness)
{
	return Colors::shade(QGuiApplication::palette().color(QPalette::Button), light_theme_lightness, dark_theme_lightness);
}

QColor Colors::relativeButtonShade(float relative_lightness)
{
	return Colors::relativeShade(QGuiApplication::palette().color(QPalette::Button), relative_lightness);
}

QColor Colors::textShade(int light_theme_lightness, int dark_theme_lightness)
{
	return Colors::shade(QGuiApplication::palette().color(QPalette::ButtonText), light_theme_lightness, dark_theme_lightness);
}

QColor Colors::relativeTextShade(float relative_lightness)
{
	return Colors::relativeShade(QGuiApplication::palette().color(QPalette::ButtonText), relative_lightness);
}

