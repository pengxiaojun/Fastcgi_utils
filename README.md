Provide different module configuration base on web 

Specification:

Client Request: GET/POST
Server Response: json

Client request specification:

for GET: required query parameter:

target: nvr/tvw.
        sample: http://192.168.1.45/main.fcgi?target=nvr


for POST: required body parameter:

Post data format: JSON

client sample javascrpit:

var postdata = {
	"target" : nvr,
	"action" : "add",
	"nvrs" : [
		{"id", 1, "name" : "nvr1"},
		{"id", 2, "name" : "nvr2"}
	]
};

var postdata_json = JSON.stringify(postdata);
$.ajax({
	url : "webadmin.fcgi",
	dataType : "json",
	type : "post",
	data : postdata_json,
	success : function(data){
		alert("success");
	}
});

server sample code:
void handle_request(const char *postdata)
{
	json_object_t obj = json.parse(postdata);

	if (obj["action"] = "add"){
		.....
		//send response json data to client
		const char *resp = "{\"retcode\" : 0}";
		send_response(resp);
	}
}

