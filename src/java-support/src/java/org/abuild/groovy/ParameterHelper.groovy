package org.abuild.groovy

import org.abuild.groovy.Parameterized

class ParameterHelper
{
    private Parameterized _p
    private String _name

    public ParameterHelper(Parameterized p)
    {
        this._p = p
        this._name = ''
    }

    private ParameterHelper(Parameterized p, String name)
    {
        this._p = p
        this._name = name
    }

    private String child(String property)
    {
        (_name ? "${_name}." : '') + property
    }

    public Object get(String property)
    {
        return new ParameterHelper(this._p, child(property))
    }

    public void set(String property, Object value)
    {
        if (value instanceof ParameterHelper)
        {
            _p.setParameter(child(property), _p.resolve(value._name))
        }
        else
        {
            _p.setParameter(child(property), value)
        }
    }
    public void leftShift(Object value)
    {
        if (value instanceof ParameterHelper)
        {
            _p.appendParameter(_name, _p.resolve(value._name))
        }
        else
        {
            _p.appendParameter(_name, value)
        }
    }

    public void delete(ParameterHelper ph)
    {
        _p.deleteParameter(ph._name)
    }
}
