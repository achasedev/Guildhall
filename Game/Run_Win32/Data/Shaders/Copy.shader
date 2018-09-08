<shader>

    <program name="Copy">
    	<vertex file="Data/ShaderPrograms/Copy.vs"/>
    	<fragment file="Data/ShaderPrograms/Copy.fs"/>
    </program>

    <cull mode="back"/>
    <fill mode="solid"/>
    <wind order="ccw"/>
    <depth test="always" write="false"/>
    <blend>
    	<color op="add" source="one" destination="zero"/>
    	<alpha op="add" source="one" destination="one"/>
    </blend>

</shader>