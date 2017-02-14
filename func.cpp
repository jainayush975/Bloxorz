void renderBrick(VAO *object, glm::mat4 VP){
  int i = state.pos[0][0], j = state.pos[0][1];
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleObject;
  if(state.status==1)
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(10, 20, 10));
  else if(state.pos[0][0]==state.pos[1][0])
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 20));
  else if(state.pos[0][1]==state.pos[1][1])
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(20, 10, 10));
  glm::mat4 trans1,trans2,rot1;
  if(state.status==1){
    switch(state.rotDirection){
      case 'B':
        trans1 = glm::translate (glm::vec3(0,10.0,5.0));
        rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
        trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-50));
        break;
      case 'F':
        trans1 = glm::translate (glm::vec3(0,10.0,-5.0));
        rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
        trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-40));
        break;
      case 'L':
        trans1 = glm::translate (glm::vec3(5.0,10.0,0));
        rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
        trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-45));
        break;
      case 'R':
        trans1 = glm::translate (glm::vec3(-5.0,10.0,0));
        rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
        trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-45));
        break;
      default:
        trans1 = glm::mat4(1.0f);
        rot1 = glm::mat4(1.0f);
        trans2 = glm:: translate(glm::vec3(i*10-45,10,j*10-45));
        break;
      }
  }
if(state.pos[0][0]==state.pos[1][0]){
  //cout << state.rotDirection << endl;
  switch(state.rotDirection){
    case 'F':
      trans1 = glm::translate (glm::vec3(0,5.0,-10.0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-30));
      break;
    case 'B':
      trans1 = glm::translate (glm::vec3(0,5.0,10.0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-50));
      break;
    case 'R':
      trans1 = glm::translate (glm::vec3(-5.0,5.0,0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-40));
      break;
    case 'L':
      trans1 = glm::translate (glm::vec3(5.0,5.0,0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-40));
      break;
    default:
      trans1 = glm::mat4(1.0f);
      rot1 = glm::mat4(1.0f);
      trans2 = glm:: translate(glm::vec3(i*10-45,5,j*10-40));
      break;
    }
}
if(state.pos[0][1]==state.pos[1][1]){
  switch(state.rotDirection){
    case 'F':
      trans1 = glm::translate (glm::vec3(0,5.0,-5.0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-40));
      break;
    case 'B':
      trans1 = glm::translate (glm::vec3(0,5.0,5.0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-50));
      break;
    case 'R':
      trans1 = glm::translate (glm::vec3(-10.0,5.0,0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-30, 0, j*10-45));
      break;
    case 'L':
      trans1 = glm::translate (glm::vec3(10.0,5.0,0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-45));
      break;
    default:
      trans1 = glm::mat4(1.0f);
      rot1 = glm::mat4(1.0f);
      trans2 = glm:: translate(glm::vec3(i*10-40,5,j*10-45));
      break;
    }
}

  glm::mat4 objectTransform = trans2 * rot1 * trans1 * scaleObject;
  Matrices.model *= objectTransform;
  glm::mat4 MVP = VP * Matrices.model; // MVP = p * V * M

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(object);
}
