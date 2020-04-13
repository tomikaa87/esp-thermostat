/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2018-09-30
*/

#pragma once

template <typename T>
struct TrackedVariable
{
    TrackedVariable() = default;
    TrackedVariable(const T value)
        : m_value{ value }
    {}

    bool changed() const
    {
        return m_changed;
    }

    TrackedVariable& operator=(const T value)
    {
        if (m_value == value)
            return *this;

        m_value = value;
        m_changed = true;

        return *this;
    }

    operator T()
    {
        return value();
    }

    T value()
    {
        m_changed = false;
        return m_value;
    }

private:
    T m_value = {};
    bool m_changed = false;
};
