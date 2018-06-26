<shader>

    <program name="Water">
    	<vertex file="Data/ShaderPrograms/Water.vs"/>
    	<fragment file="Data/ShaderPrograms/Water.fs"/>
    </program>

    <cull mode="none"/>
    <fill mode="solid"/>
    <wind order="ccw"/>
    <depth test="less" write="false"/>
    <blend>
    	<color op="add" source="source_alpha" destination="one_minus_source_alpha"/>
    	<alpha op="add" source="one" destination="one"/>
    </blend>

    <order layer="0" queue="alpha"/>

</shader>