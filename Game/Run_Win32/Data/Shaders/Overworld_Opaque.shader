<shader>

	<program name="Overworld_Opaque">

		<vertex file="Data/ShaderPrograms/Overworld_Opaque.vs"/>
		<fragment file="Data/ShaderPrograms/Overworld_Opaque.fs"/>

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