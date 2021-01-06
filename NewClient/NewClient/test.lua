Roy_protocol = Proto("Roy","ROY")

msg_types ={
[1] = "RoY Request",
[2] = "RoY Response",
}

roy_fields = Roy_protocol.fields
roy_fields.p_type = ProtoField.uint8("Roy.Type","Type",base.DEC,msg_types)
roy_fields.flag = ProtoField.uint8("Roy.Flag","Flag",base.DEC)
roy_fields.seq = ProtoField.uint8("Roy.Seq","Seq",base.HEX)
roy_fields.my_data_req = ProtoField.uint16("Roy.Request","Request",base.HEX)
roy_fields.my_data_res = ProtoField.uint16("Roy.Response","Response",base.HEX)
roy_fields.check_it = ProtoField.uint8("Roy.Check_it","Check_it",base.DEC)

function func_add_header(headerSubtree,buffer,offset)
	headerSubtree:add(roy_fields.ptype,buffer(offset,1))
	headerSubtree:add(roy_fields.flag,buffer(offset+1,1))
	headerSubtree:add(roy_fields.seq,buffer(offset+2,1))
	offset = offset + 3
	return offset
end

function func_add_payload(subtree,buffer,offset,p_type)
	local payload_msg = "Payload"
	
	if p_type == 1 then
		payloadSubtree = subtree:add(Roy_protocol , buffer(offset,2) , payload_msg)
		payloadSubtree:add(roy_fields.my_data_req , buffer(offset,2))
		offset = offset + 2
	end
	
	if p_type == 2 then
		payloadSubtree = subtree:add(Roy_protocol, buffer(offset,3) , payload_msg)
		payloadSubtree:add(roy_fields.check_it , buffer(offset,1))
		payloadSubtree:add(roy_fields.my_data_res , buffer(offset+1,2))
		offset = offset + 3
	end
	
	return offset
end

function func_add_ports()

	tcp_table = DissectorTable.get("tcp.port")
	tcp_table:add(79,Roy_protocol)
	tcp_table:add(80,Roy_protocol)
	tcp_table:add(81,Roy_protocol)
	tcp_table:add(82,Roy_protocol)
	tcp_table:add(83,Roy_protocol)
	tcp_table:add(84,Roy_protocol)
	tcp_table:add(85,Roy_protocol)
	tcp_table:add(86,Roy_protocol)
	tcp_table:add(87,Roy_protocol)
end
	

function Roy_protocol.dissector(buffer,pinfo,tree)
	pinfo.cols['protocol'] = Roy_protocol.name
	                                                  
	local subtree = tree:add(Roy_protocol,buffer())
	local offset = 0
	local headerSubtree = subtree:add(Roy_protocol,buffer(offset,3),"Header")
	
	local p_type = buffer(offset,1):uint()
	pinfo.cols['info'] = msg_types[p_type]
	
	offset = func_add_header(headerSubtree,buffer(),offset)
	offset = func_add_payload(subtree,buffer(),offset,p_type)
	
end

func_add_ports()


