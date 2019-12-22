# 1. Install client with npm
FROM node:12.10-alpine as build-stage
WORKDIR /client
COPY ./package*.json /client/
RUN npm install
COPY ./ /client/
# Build static content
RUN npm run build

# 2. Copy client files into nginx container and run nginx on port 80
FROM nginx:1.17-alpine

#copy nginx config
COPY --from=build-stage /client/nginx.conf /etc/nginx/nginx.conf

#copy static client content
COPY --from=build-stage /client/build/ /usr/share/nginx/html

EXPOSE 80

# 3. Run the backend server
WORKDIR /server

#copy dynamic server content
COPY server/ /server

RUN apk add --update nodejs npm
RUN npm install
RUN npm install -g forever

# run forever
CMD sh start.sh