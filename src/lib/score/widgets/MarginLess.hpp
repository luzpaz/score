#pragma once

#include <QWidget>

namespace score
{
/**
 * @brief The MarginLess class
 *
 * A mixin that removes the margin of a layout.
 */
template <typename Layout>
class MarginLess final : public Layout
{
public:
  MarginLess(QWidget* widg) : Layout{widg}
  {
    this->setContentsMargins(2, 2, 2, 2);
    this->setSpacing(2);
    widg->setLayout(this);
  }
  MarginLess()
  {
    this->setContentsMargins(2, 2, 2, 2);
    this->setSpacing(2);
  }
};
}
