<shader>

    <program name="Tank">
    	<vertex file="Data/ShaderPrograms/Tank.vs"/>
    	<fragment file="Data/ShaderPrograms/Tank.fs"/>
    </program>

    <cull mode="back"/>
    <fill mode="solid"/>
    <wind order="ccw"/>
    <depth test="less"/>
    <blend>
    	<color op="add" source="one" destination="zero"/>
    	<alpha op="add" source="one" destination="one"/>
    </blend>

</shader>