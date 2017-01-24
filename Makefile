.PHONY: run_container share_container

run_container:
	docker build -t zmq-hwm-test .
	docker run -it --rm \
	-v $$PWD:/host \
	-w /host/build \
	zmq-hwm-test

share_container:
	docker exec -it \
	$$(docker ps -f 'ancestor=zmq-hwm-test' --format '{{.ID}}') \
	/bin/bash
