<shader>

    <program name="Voxel">
    	<vertex file="Data/ShaderPrograms/Voxel.vs"/>
    	<fragment file="Data/ShaderPrograms/Voxel.fs"/>
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