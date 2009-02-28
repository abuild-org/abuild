package org.abuild.javabuilder

class ParameterBuilder extends BuilderSupport
{
    Map<String, Object> parameters = [:]

    def createNode(name)
    {
        createNode(name, null, null)
    }

    def createNode(name, value)
    {
        createNode(name, null, value)
    }

    def createNode(name, Map attributes)
    {
        createNode(name, attributes, null)
    }

    def createNode(name, Map attributes, value)
    {
        String key = current

        if (key == null)
        {
            if (name == 'call')
            {
                key = ''
            }
            else
            {
                key = name
            }
        }
        else
        {
            key = (key ? "${key}." : "") + name
        }
        if (! ((value == null) && (attributes == null)))
        {
            if (key == '')
            {
                throw new Exception("attempt to set empty parameter")
            }
            if ((! parameters.containsKey(key)) && (attributes == null))
            {
                parameters[key] = value
            }
            else
            {
                if (parameters.containsKey(key))
                {
                    if (! (parameters[key] instanceof List))
                    {
                        parameters[key] = [parameters[key]]
                    }
                }
                else
                {
                    parameters[key] = []
                }

                if (attributes != null)
                {
                    parameters[key] << attributes
                }
                if (value != null)
                {
                    parameters[key] << value
                }
            }
        }

        key
    }

    void setParent(parent, child)
    {
    }
}
