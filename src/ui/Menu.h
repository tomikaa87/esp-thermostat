#pragma once

#include "Graphics.h"
#include "Resources.h"
#include "Types.h"

#include <array>
#include <string_view>
#include <variant>

namespace UI
{

namespace
{
    constexpr auto MaxMenuItemCount{ 10 };
}

struct MenuItem
{
    std::string_view text;
};

template <typename GraphicsType, std::size_t ItemCount>
class MenuImpl
{
public:
    template <typename... Items>
    explicit MenuImpl(GraphicsType& display, const int startLine, const int height, Items&&... items)
        : _graphics{ display }
        , _items{ std::forward<Items>(items)... }
        , _startLine{ startLine }
        , _height{ height }
    {
    }

    void update()
    {
        drawItems();
    }

    void step(const StepDirection direction)
    {
        switch (direction) {
            case StepDirection::Up:
                if (_selectionIndex == 0) {
                    _selectionIndex = _items.size() - 1;
                    if (_items.size() > _height) {
                        _viewPosition = std::max(_items.size() - _height, 0u);
                    }
                } else {
                    --_selectionIndex;
                    if (_selectionIndex < _viewPosition) {
                        _viewPosition = _selectionIndex;
                    }
                }

                break;

            case StepDirection::Down:
                ++_selectionIndex;
                if (_selectionIndex == _items.size()) {
                    _selectionIndex = 0;
                    _viewPosition = 0;
                } else if (_selectionIndex >= _height) {
                    ++_viewPosition;
                }

                break;
        }
    }

    [[nodiscard]] int currentIndex() const
    {
        return _selectionIndex;
    }

private:
    GraphicsType& _graphics;
    std::array<MenuItem, ItemCount> _items;
    const int _startLine;
    const int _height;
    int _selectionIndex{};
    int _viewPosition{};

    void drawItems()
    {
        auto itemIndex{ _viewPosition };
        auto line{ _startLine };

        while (
            itemIndex < _items.size()
            && line < (_startLine + _height)
            && line <= (GraphicsType::Lines - 1)
        ) {
            if (itemIndex == _selectionIndex) {
                _graphics.drawBitmap(0, line, Resources::Assets::ArrowRightIcon);
            } else {
                _graphics.fillArea(0, line, sizeof(Resources::Assets::ArrowRightIcon), 1, Graphics::Color::Black);
            }

            auto x = _graphics.drawText(
                sizeof(Resources::Assets::ArrowRightIcon) + 2,
                line,
                _items[itemIndex].text,
                Resources::Fonts::Oled
            );

            if (x < (GraphicsType::Width - 1 - sizeof(Resources::Assets::EmptyPositionIndicator))) {
                _graphics.fillArea(
                    x,
                    line,
                    GraphicsType::Width - 1 - x - sizeof(Resources::Assets::EmptyPositionIndicator),
                    1,
                    Graphics::Color::Black
                );
            }

            uint8_t position = _selectionIndex == 0 ? 0 : ((_selectionIndex + 1) * (_height - 1) / _items.size());

            for (uint8_t i = 0; i <= (_height - 1); ++i) {
                if (i == position) {
                    _graphics.drawBitmap(
                        GraphicsType::Width - sizeof(Resources::Assets::FullPositionIndicator) - 1,
                        i + _startLine,
                        Resources::Assets::FullPositionIndicator
                    );
                } else {
                    _graphics.drawBitmap(
                        GraphicsType::Width - sizeof(Resources::Assets::EmptyPositionIndicator) - 1,
                        i + _startLine,
                        Resources::Assets::EmptyPositionIndicator
                    );
                }
            }

            ++line;
            ++itemIndex;
        }
    }
};

class Menu
{
public:
    template <typename GraphicsType, typename... Item>
    Menu(GraphicsType& graphics, const int startLine, const int height, Item&&... items)
        : _menu{
            MenuImpl<GraphicsType, sizeof...(Item)>{
                graphics,
                startLine,
                height,
                std::forward<Item>(items)...
            }
        }
    {
        static_assert((std::same_as<Item, MenuItem> && ...));
    }

    void update()
    {
        std::visit(
            []<typename MenuType>(MenuType& m) {
                m.update();
            },
            _menu
        );
    }

    void step(const StepDirection direction)
    {
        std::visit(
            [direction]<typename MenuType>(MenuType& menu) {
                menu.step(direction);
            },
            _menu
        );
    }

    [[nodiscard]] int currentIndex() const
    {
        return std::visit(
            []<typename MenuType>(const MenuType& menu) {
                return menu.currentIndex();
            },
            _menu
        );
    }

private:
    struct Detail
    {
        template <typename T, typename U>
        struct VariantGenerator;

        template <typename GraphicsType, std::size_t... Counts>
        struct VariantGenerator<GraphicsType, std::index_sequence<Counts...>> {
            using Variant = std::variant<MenuImpl<GraphicsType, Counts + 1>...>;
        };

        template <typename GraphicsType, std::size_t Count>
        struct GenerateVariantForMaxItemCount {
            using Type = typename VariantGenerator<
                GraphicsType,
                std::make_index_sequence<Count>
            >::Variant;
        };
    };

    using MenuVariant = Detail::GenerateVariantForMaxItemCount<
        Graphics,
        MaxMenuItemCount
    >::Type;

    MenuVariant _menu;
};

}