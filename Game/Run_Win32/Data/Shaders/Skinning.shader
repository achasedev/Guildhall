<shader>

    <program name="Skinning">
    	<vertex file="Data/ShaderPrograms/Skinning.vs"/>
    	<fragment file="Data/ShaderPrograms/Skinning.fs"/>
    </program>

    <cull mode="back"/>
    <fill mode="solid"/>
    <wind order="ccw"/>
    <depth test="less" write="true"/>
    <blend>
    	<color op="add" source="one" destination="zero"/>
    	<alpha op="add" source="one" destination="one"/>
    </blend>

</shader>