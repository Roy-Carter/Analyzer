--TEST.LUA
Roy_protocol = Proto("Roy","ROY PROTOCOL")
roy_fields = Roy_protocol.fields
roy_fields.ptype = ProtoField.uint8("Roy.Type","Type",base.DEC)
roy_fields.flag = ProtoField.uint8("Roy.Flag","Flag",base.DEC)
roy_fields.seq = ProtoField.uint8("Roy.Seq","Seq",base.HEX)
roy_fields.my_data_req = ProtoField.uint16("Roy.Accept","Accept",base.HEX)
roy_fields.my_data_res = ProtoField.uint16("Roy.Response","Response",base.HEX)


function func_add_header(headerSubtree,buffer)
	headerSubtree:add_le(roy_fields.ptype,buffer(0,1))
	
	headerSubtree:add_le(roy_fields.flag,buffer(1,1))
	
	headerSubtree:add_le(roy_fields.seq,buffer(2,1))
end

function func_request(subtree,buffer,offset)
	
	offset = add_field(subtree, buffer, roy_fields.my_data_req, offset, 1)
	
	return offset
end


function Roy_protocol.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = Roy_protocol.name
	pinfo.cols.info = "Roy's Protocol"
	                                                  
	local p_type = buffer(0,1):uint()
	local subtree = tree:add(Roy_protocol,buffer(),"Roy's PROTOCOL Info")
	
	local headerSubtree = subtree:add(Roy_protocol,buffer(0,3),"Roy's Header")
	
	func_add_header(headerSubtree,buffer())
	
	local offset = 0
	local payloadSubtree
	local payload_type 
	
	if p_type == 0x1 then
		payload_type = "Payload Type : REQUEST = 1"
		payloadSubtree = subtree:add(Roy_protocol,buffer(3),payload_type)
		payloadSubtree:add_le(roy_fields.my_data_req,buffer(3))
	elseif p_type == 0x2 then
		payload_type = "Payload Type : RESPONSE = 2"
		payloadSubtree = subtree:add(Roy_protocol,buffer(3),payload_type)
		payloadSubtree:add_le(roy_fields.my_data_res,buffer(3))
	else
		payload_type = "Payload Type : FINISH = 3"
	
	end
	

	
end

tcp_table = DissectorTable.get("tcp.port")
tcp_table:add(79,Roy_protocol)
tcp_table:add(80,Roy_protocol)



	
	